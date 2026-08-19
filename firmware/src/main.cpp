/**
 * @file main.cpp
 * @brief Headless SenseESP foundation for the Signal K BLE Gateway.
 *
 * Runs the generic Advertisement MVP: passive BLE scanning, bounded batching
 * and versioned HTTP delivery to the server-side gateway provider.
 */

#include <Arduino.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp32-hal-bt-mem.h>

#include <memory>

#include "advertisement_gateway.h"
#include "secrets.h"
#include "sensesp.h"
#include "sensesp_app_builder.h"
#include "sensesp/ui/status_page_item.h"
#include "sensesp_ble_gateway/native_bluedroid_ble.h"

using namespace sensesp;

namespace {

constexpr char kLogTag[] = "gateway";
constexpr uint32_t kHeartbeatIntervalMs = 30000;

#ifndef WEB_ADMIN_USERNAME
#define WEB_ADMIN_USERNAME "admin"
#endif

#ifndef WEB_ADMIN_PASSWORD
#define WEB_ADMIN_PASSWORD OTA_PASSWORD
#endif

#ifndef GATEWAY_ID
#define GATEWAY_ID HOSTNAME
#endif

StatusPageItem<String> firmware_version{"Gateway firmware", FIRMWARE_VERSION,
                                        "Software", 3350};
StatusPageItem<String> firmware_build{"Gateway build", FIRMWARE_BUILD_ID,
                                      "Software", 3360};
StatusPageItem<String> foundation_state{
    "Gateway phase", "advertisement MVP", "Gateway", 4000};
StatusPageItem<String> ble_state{"BLE transport", "initializing", "Gateway",
                                 4010};

std::shared_ptr<NativeBLE> ble;
std::shared_ptr<gateway::AdvertisementGateway> advertisement_gateway;
String ble_startup_error;

const char* controller_status_name(esp_bt_controller_status_t status) {
  switch (status) {
    case ESP_BT_CONTROLLER_STATUS_IDLE:
      return "idle";
    case ESP_BT_CONTROLLER_STATUS_INITED:
      return "initialized";
    case ESP_BT_CONTROLLER_STATUS_ENABLED:
      return "enabled";
    default:
      return "unknown";
  }
}

const char* host_status_name(esp_bluedroid_status_t status) {
  switch (status) {
    case ESP_BLUEDROID_STATUS_UNINITIALIZED:
      return "uninitialized";
    case ESP_BLUEDROID_STATUS_INITIALIZED:
      return "initialized";
    case ESP_BLUEDROID_STATUS_ENABLED:
      return "enabled";
    default:
      return "unknown";
  }
}

bool reset_preinitialized_bluetooth() {
  const esp_bluedroid_status_t bluedroid_status =
      esp_bluedroid_get_status();
  const esp_bt_controller_status_t initial_controller_status =
      esp_bt_controller_get_status();
  ESP_LOGI(kLogTag, "Bluetooth before normalization: host=%d controller=%d",
           static_cast<int>(bluedroid_status),
           static_cast<int>(initial_controller_status));
  if (bluedroid_status == ESP_BLUEDROID_STATUS_ENABLED) {
    const esp_err_t result = esp_bluedroid_disable();
    if (result != ESP_OK) {
      ble_startup_error = "host disable: " + String(esp_err_to_name(result));
      ESP_LOGE(kLogTag, "%s", ble_startup_error.c_str());
      return false;
    }
  }
  if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_INITIALIZED) {
    const esp_err_t result = esp_bluedroid_deinit();
    if (result != ESP_OK) {
      ble_startup_error = "host deinit: " + String(esp_err_to_name(result));
      ESP_LOGE(kLogTag, "%s", ble_startup_error.c_str());
      return false;
    }
  }

  const esp_bt_controller_status_t controller_status =
      esp_bt_controller_get_status();
  if (controller_status == ESP_BT_CONTROLLER_STATUS_ENABLED) {
    const esp_err_t result = esp_bt_controller_disable();
    if (result != ESP_OK) {
      ble_startup_error =
          "controller disable: " + String(esp_err_to_name(result));
      ESP_LOGE(kLogTag, "%s", ble_startup_error.c_str());
      return false;
    }
  }
  if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED) {
    const esp_err_t result = esp_bt_controller_deinit();
    if (result != ESP_OK) {
      ble_startup_error =
          "controller deinit: " + String(esp_err_to_name(result));
      ESP_LOGE(kLogTag, "%s", ble_startup_error.c_str());
      return false;
    }
  }

  // ESP-IDF changes the public state to IDLE before all controller cleanup has
  // completed. Arduino's own btStop() waits a scheduler tick for this reason;
  // leave a slightly larger margin before NativeBLE initializes it again.
  vTaskDelay(pdMS_TO_TICKS(10));

  const esp_bt_controller_status_t final_controller_status =
      esp_bt_controller_get_status();
  ESP_LOGI(kLogTag, "Bluetooth after normalization: host=%d controller=%d",
           static_cast<int>(esp_bluedroid_get_status()),
           static_cast<int>(final_controller_status));
  if (final_controller_status != ESP_BT_CONTROLLER_STATUS_IDLE) {
    ble_startup_error = "controller did not reach idle";
    return false;
  }
  return true;
}

void log_heartbeat() {
  const uint32_t ble_hits = ble ? ble->scan_hit_count() : 0;
  const bool scanning = ble && ble->is_scanning();
  const uint32_t received =
      advertisement_gateway ? advertisement_gateway->received() : 0;
  const uint32_t delivered =
      advertisement_gateway ? advertisement_gateway->delivered() : 0;
  const uint32_t dropped =
      advertisement_gateway ? advertisement_gateway->dropped() : 0;
  const size_t pending =
      advertisement_gateway ? advertisement_gateway->pending() : 0;
  const uint32_t dropped_queue = advertisement_gateway
                                     ? advertisement_gateway->dropped_queue_full()
                                     : 0;
  const uint32_t dropped_lock = advertisement_gateway
                                    ? advertisement_gateway->dropped_lock_timeout()
                                    : 0;
  const uint32_t dropped_invalid =
      advertisement_gateway
          ? advertisement_gateway->dropped_invalid_batch()
          : 0;
  const auto controller_status = esp_bt_controller_get_status();
  const auto host_status = esp_bluedroid_get_status();
  const uint32_t scan_requests = advertisement_gateway
                                     ? advertisement_gateway->scan_start_requests()
                                     : 0;
  const uint32_t scan_failures = advertisement_gateway
                                     ? advertisement_gateway->scan_start_failures()
                                     : 0;
  const bool token_available =
      advertisement_gateway && advertisement_gateway->token_available();

  String state = scanning ? "scanning" : "stopped";
  if (!ble_startup_error.isEmpty()) {
    state = "error: " + ble_startup_error;
  }
  ble_state.set(state +
                ", controller=" + controller_status_name(controller_status) +
                ", host=" + host_status_name(host_status) +
                ", starts=" + String(scan_requests) +
                ", start_failures=" + String(scan_failures) +
                ", token=" + (token_available ? "available" : "missing") +
                ", received=" + String(received) +
                ", delivered=" + String(delivered) +
                ", pending=" + String(pending) +
                ", dropped=" + String(dropped) +
                ", drop_queue=" + String(dropped_queue) +
                ", drop_lock=" + String(dropped_lock) +
                ", drop_invalid=" + String(dropped_invalid) +
                ", post_ok=" +
                String(advertisement_gateway
                           ? advertisement_gateway->post_success()
                           : 0) +
                ", post_fail=" +
                String(advertisement_gateway
                           ? advertisement_gateway->post_fail()
                           : 0) +
                ", http_status=" +
                String(advertisement_gateway
                           ? advertisement_gateway->last_http_status()
                           : 0) +
                ", post_ms=" +
                String(advertisement_gateway
                           ? advertisement_gateway->last_post_duration_ms()
                           : 0) +
                ", retry_ms=" +
                String(advertisement_gateway
                           ? advertisement_gateway->retry_interval_ms()
                           : 0));

  ESP_LOGI(kLogTag,
           "alive uptime=%lus heap=%u min_heap=%u ble_scan=%d ble_hits=%u "
           "controller=%s host=%s scan_starts=%u scan_start_fail=%u token=%d "
           "received=%u delivered=%u pending=%u dropped=%u post_ok=%u "
           "post_fail=%u drop_queue=%u drop_lock=%u drop_invalid=%u "
           "http_status=%d post_ms=%u retry_ms=%u build=%s",
           static_cast<unsigned long>(millis() / 1000),
           static_cast<unsigned>(ESP.getFreeHeap()),
           static_cast<unsigned>(ESP.getMinFreeHeap()), scanning,
           static_cast<unsigned>(ble_hits),
           controller_status_name(controller_status),
           host_status_name(host_status), static_cast<unsigned>(scan_requests),
           static_cast<unsigned>(scan_failures), token_available,
           static_cast<unsigned>(received),
           static_cast<unsigned>(delivered), static_cast<unsigned>(pending),
           static_cast<unsigned>(dropped),
           static_cast<unsigned>(advertisement_gateway
                                     ? advertisement_gateway->post_success()
                                     : 0),
           static_cast<unsigned>(advertisement_gateway
                                     ? advertisement_gateway->post_fail()
                                     : 0),
           static_cast<unsigned>(dropped_queue),
           static_cast<unsigned>(dropped_lock),
           static_cast<unsigned>(dropped_invalid),
           static_cast<int>(advertisement_gateway
                                ? advertisement_gateway->last_http_status()
                                : 0),
           static_cast<unsigned>(
               advertisement_gateway
                   ? advertisement_gateway->last_post_duration_ms()
                   : 0),
           static_cast<unsigned>(advertisement_gateway
                                     ? advertisement_gateway->retry_interval_ms()
                                     : 0),
           FIRMWARE_BUILD_ID);
}

}  // namespace

void setup() {
  SetupLogging(ESP_LOG_INFO);
  ESP_LOGI(kLogTag, "starting firmware=%s", FIRMWARE_VERSION);

  SensESPAppBuilder builder;
  auto app = builder.set_hostname(HOSTNAME)
      ->set_wifi_client(WIFI_SSID, WIFI_PASSWORD)
      // Disable the fallback AP after credentials have been provisioned. This
      // avoids exposing a second management network in normal operation.
      ->set_wifi_access_point("", "")
      ->set_sk_server(SK_SERVER, SK_PORT)
      ->set_admin_user(WEB_ADMIN_USERNAME, WEB_ADMIN_PASSWORD)
      ->enable_ota(OTA_PASSWORD)
      // Recovery-button behavior depends on the final board and enclosure.
      // Disable it until a safe GPIO and physical interaction are specified.
      ->set_button_pin(-1)
      ->get_app();

  if (!reset_preinitialized_bluetooth()) {
    ble_state.set("error: " + ble_startup_error);
    ESP_LOGE(kLogTag, "Bluetooth controller normalization failed: %s",
             ble_startup_error.c_str());
  } else {
    NativeBLEConfig ble_config;
    ble_config.active_scan = false;
    ble_config.scan_interval_ms = 100;
    ble_config.scan_window_ms = 50;
    ble = std::make_shared<NativeBLE>(ble_config);
    ESP_LOGI(kLogTag,
             "NativeBLE constructed: controller=%s host=%s mac=%s heap=%u",
             controller_status_name(esp_bt_controller_get_status()),
             host_status_name(esp_bluedroid_get_status()),
             ble->mac_address().c_str(), static_cast<unsigned>(ESP.getFreeHeap()));
  }

  gateway::AdvertisementGatewayConfig gateway_config;
  gateway_config.gateway_id = GATEWAY_ID;
  gateway_config.gateway_mac = ble ? ble->mac_address() : "";
  gateway_config.hostname = HOSTNAME;
  gateway_config.firmware_version = FIRMWARE_VERSION;
#ifdef GATEWAY_PROVIDER_TOKEN
  gateway_config.provider_token = GATEWAY_PROVIDER_TOKEN;
#endif
  advertisement_gateway = std::make_shared<gateway::AdvertisementGateway>(
      ble, app->get_ws_client(), gateway_config);
  if (!advertisement_gateway->start()) {
    if (ble_startup_error.isEmpty()) {
      ble_startup_error = "gateway startup failed";
    }
    ble_state.set("error: " + ble_startup_error);
    ESP_LOGE(kLogTag, "Advertisement gateway startup failed");
  }

  event_loop()->onRepeat(kHeartbeatIntervalMs, log_heartbeat);
  log_heartbeat();
}

void loop() { event_loop()->tick(); }
