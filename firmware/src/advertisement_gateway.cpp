#include "advertisement_gateway.h"

#include <ArduinoJson.h>
#include <esp_http_client.h>
#include <esp_log.h>

#include <algorithm>

namespace gateway {
namespace {

constexpr char kLogTag[] = "gateway_http";
constexpr char kAdvertisementsPath[] =
    "/signalk/v2/api/ble/gateway/advertisements";
constexpr uint32_t kHttpTimeoutMs = 5000;
constexpr uint32_t kScanRecoveryIntervalMs = 10000;
constexpr uint32_t kMissingTokenLogIntervalMs = 30000;
constexpr uint32_t kHttpTaskStackSize = 6144;

String bytes_to_hex(const std::vector<uint8_t>& bytes) {
  String result;
  result.reserve(bytes.size() * 2);
  for (const uint8_t byte : bytes) {
    char encoded[3];
    snprintf(encoded, sizeof(encoded), "%02X", byte);
    result += encoded;
  }
  return result;
}

const char* address_type_name(const uint8_t address_type) {
  switch (address_type) {
    case 0:
      return "public";
    case 1:
      return "random";
    default:
      return nullptr;
  }
}

}  // namespace

AdvertisementGateway::AdvertisementGateway(
    std::shared_ptr<sensesp::BLEProvisioner> ble,
    std::shared_ptr<sensesp::SKWSClient> signalk_client,
    AdvertisementGatewayConfig config)
    : ble_(std::move(ble)),
      signalk_client_(std::move(signalk_client)),
      config_(std::move(config)) {
  mutex_ = xSemaphoreCreateMutex();
  pending_.reserve(config_.max_pending_advertisements);
  inflight_.reserve(config_.max_batch_size);
}

AdvertisementGateway::~AdvertisementGateway() {
  stop();
  if (mutex_ != nullptr) {
    vSemaphoreDelete(mutex_);
    mutex_ = nullptr;
  }
}

bool AdvertisementGateway::start() {
  if (running_.exchange(true)) {
    return true;
  }
  if (!ble_) {
    ESP_LOGE(kLogTag, "Invalid gateway configuration: BLE is null");
    running_.store(false);
    return false;
  }
  if (!signalk_client_) {
    ESP_LOGE(kLogTag, "Invalid gateway configuration: SK client is null");
    running_.store(false);
    return false;
  }
  if (mutex_ == nullptr || config_.gateway_id.isEmpty() ||
      config_.max_batch_size == 0 ||
      config_.max_pending_advertisements == 0) {
    ESP_LOGE(kLogTag,
             "Invalid gateway configuration: mutex=%d gateway_id=%d "
             "batch=%u pending=%u",
             mutex_ != nullptr, !config_.gateway_id.isEmpty(),
             static_cast<unsigned>(config_.max_batch_size),
             static_cast<unsigned>(config_.max_pending_advertisements));
    running_.store(false);
    return false;
  }

  ble_->attach([this]() { on_advertisement(); });
  if (xTaskCreate(task_entry, "gateway_http", kHttpTaskStackSize, this, 1,
                  &task_) !=
      pdPASS) {
    ESP_LOGE(kLogTag, "Could not create HTTP transport task");
    running_.store(false);
    return false;
  }
  scan_start_requests_.fetch_add(1, std::memory_order_relaxed);
  if (!ble_->start_scan()) {
    scan_start_failures_.fetch_add(1, std::memory_order_relaxed);
    ESP_LOGE(kLogTag, "Could not start BLE scan");
    stop();
    return false;
  }

  ESP_LOGI(kLogTag, "Started official BLE API gateway_id=%s",
           config_.gateway_id.c_str());
  return true;
}

void AdvertisementGateway::stop() {
  if (!running_.exchange(false)) {
    return;
  }
  if (ble_ && ble_->is_scanning()) {
    ble_->stop_scan();
  }
  // The task observes running_ and deletes itself. Avoid deleting it while it
  // owns the queue mutex or is inside the HTTP client.
  task_ = nullptr;
}

size_t AdvertisementGateway::pending() const {
  if (mutex_ == nullptr ||
      xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) != pdTRUE) {
    return 0;
  }
  const size_t count = pending_.size() + inflight_.size();
  xSemaphoreGive(mutex_);
  return count;
}

void AdvertisementGateway::on_advertisement() {
  if (!running_.load()) {
    return;
  }
  received_.fetch_add(1, std::memory_order_relaxed);
  const sensesp::BLEAdvertisement advertisement = ble_->get();

  if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(20)) != pdTRUE) {
    dropped_.fetch_add(1, std::memory_order_relaxed);
    return;
  }
  if (pending_.size() >= config_.max_pending_advertisements) {
    pending_.erase(pending_.begin());
    dropped_.fetch_add(1, std::memory_order_relaxed);
  }
  pending_.push_back(advertisement);
  xSemaphoreGive(mutex_);
}

void AdvertisementGateway::task_entry(void* argument) {
  static_cast<AdvertisementGateway*>(argument)->task_loop();
}

void AdvertisementGateway::task_loop() {
  uint32_t retry_interval_ms = config_.post_interval_ms;
  uint32_t next_scan_recovery_ms = millis() + kScanRecoveryIntervalMs;
  uint32_t next_missing_token_log_ms = 0;

  while (running_.load()) {
    const uint32_t now = millis();
    if (!ble_->is_scanning() &&
        static_cast<int32_t>(now - next_scan_recovery_ms) >= 0) {
      const uint32_t request =
          scan_start_requests_.fetch_add(1, std::memory_order_relaxed) + 1;
      ESP_LOGW(kLogTag,
               "BLE scan is stopped; recovery start request=%u",
               static_cast<unsigned>(request));
      if (!ble_->start_scan()) {
        scan_start_failures_.fetch_add(1, std::memory_order_relaxed);
        ESP_LOGE(kLogTag,
                 "BLE scan recovery request=%u rejected synchronously",
                 static_cast<unsigned>(request));
      }
      next_scan_recovery_ms = now + kScanRecoveryIntervalMs;
    }

    const bool dedicated_token_available = !config_.provider_token.isEmpty();
    const bool shared_sk_token_available =
        !signalk_client_->get_auth_token().isEmpty();
    const bool token_available =
        dedicated_token_available || shared_sk_token_available;
    if (!token_available) {
      if (static_cast<int32_t>(now - next_missing_token_log_ms) >= 0) {
        ESP_LOGW(kLogTag,
                 "Provider delivery deferred: no bearer token available");
        next_missing_token_log_ms = now + kMissingTokenLogIntervalMs;
      }
      vTaskDelay(pdMS_TO_TICKS(config_.post_interval_ms));
      continue;
    }

    // A dedicated provider credential makes advertisement delivery independent
    // of the optional Signal K delta WebSocket. When sharing the SK client's
    // token, wait for that client to finish its authorization lifecycle.
    if (!dedicated_token_available && !signalk_client_->is_connected()) {
      vTaskDelay(pdMS_TO_TICKS(config_.post_interval_ms));
      continue;
    }

    if (!prepare_batch()) {
      vTaskDelay(pdMS_TO_TICKS(config_.post_interval_ms));
      continue;
    }

    if (post_batch()) {
      retry_interval_ms = config_.post_interval_ms;
    } else {
      retry_interval_ms =
          std::min(retry_interval_ms * 2, config_.max_retry_interval_ms);
    }
    vTaskDelay(pdMS_TO_TICKS(retry_interval_ms));
  }

  task_ = nullptr;
  vTaskDelete(nullptr);
}

bool AdvertisementGateway::prepare_batch() {
  if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) != pdTRUE) {
    return false;
  }
  if (!inflight_.empty()) {
    xSemaphoreGive(mutex_);
    return true;
  }
  if (pending_.empty()) {
    xSemaphoreGive(mutex_);
    return false;
  }

  const size_t count = std::min(pending_.size(), config_.max_batch_size);
  inflight_.assign(pending_.begin(), pending_.begin() + count);
  pending_.erase(pending_.begin(), pending_.begin() + count);
  inflight_sequence_ = next_sequence_++;
  xSemaphoreGive(mutex_);
  return true;
}

String AdvertisementGateway::serialize_batch() const {
  JsonDocument document;
  document["gateway_id"] = config_.gateway_id;
  document["firmware"] = config_.firmware_version;
  document["uptime"] = millis() / 1000;
  document["free_heap"] = ESP.getFreeHeap();
  if (!config_.gateway_mac.isEmpty()) {
    document["mac"] = config_.gateway_mac;
  }
  if (!config_.hostname.isEmpty()) {
    document["hostname"] = config_.hostname;
  }

  JsonArray devices = document["devices"].to<JsonArray>();
  for (const auto& advertisement : inflight_) {
    JsonObject device = devices.add<JsonObject>();
    device["mac"] = advertisement.address;
    const char* address_type = address_type_name(advertisement.address_type);
    if (address_type != nullptr) {
      device["address_type"] = address_type;
    }
    device["rssi"] = advertisement.rssi;
    if (!advertisement.name.isEmpty()) {
      device["name"] = advertisement.name;
    }
    device["adv_data"] = bytes_to_hex(advertisement.adv_data);
    if (!advertisement.scan_rsp_data.empty()) {
      device["scan_rsp_data"] =
          bytes_to_hex(advertisement.scan_rsp_data);
    }
  }

  String body;
  serializeJson(document, body);
  return body;
}

bool AdvertisementGateway::post_batch() {
  const String server_address = signalk_client_->get_server_address();
  const uint16_t server_port = signalk_client_->get_server_port();
  if (server_address.isEmpty() || server_port == 0) {
    return false;
  }

  const String body = serialize_batch();
  const String url = "http://" + server_address + ":" +
                     String(server_port) + kAdvertisementsPath;

  esp_http_client_config_t http_config = {};
  http_config.url = url.c_str();
  http_config.timeout_ms = kHttpTimeoutMs;
  http_config.keep_alive_enable = false;
  esp_http_client_handle_t http = esp_http_client_init(&http_config);
  if (http == nullptr) {
    ESP_LOGW(kLogTag, "Could not create HTTP client");
    return false;
  }

  esp_http_client_set_method(http, HTTP_METHOD_POST);
  esp_http_client_set_header(http, "Content-Type", "application/json");
  const String token = !config_.provider_token.isEmpty()
                           ? config_.provider_token
                           : signalk_client_->get_auth_token();
  if (!token.isEmpty()) {
    const String authorization = "Bearer " + token;
    esp_http_client_set_header(http, "Authorization",
                               authorization.c_str());
  }
  esp_http_client_set_header(http, "Connection", "close");
  esp_http_client_set_post_field(http, body.c_str(), body.length());

  const esp_err_t result = esp_http_client_perform(http);
  // esp_http_client may return an error while a valid HTTP error response is
  // available (for example a 401 challenge). Preserve both diagnostics.
  const int status = esp_http_client_get_status_code(http);
  esp_http_client_cleanup(http);

  if (status >= 200 && status < 300) {
    const size_t delivered_count = inflight_.size();
    delivered_.fetch_add(delivered_count, std::memory_order_relaxed);
    post_success_.fetch_add(1, std::memory_order_relaxed);
    ESP_LOGI(kLogTag, "Delivered batch sequence=%u advertisements=%u",
             static_cast<unsigned>(inflight_sequence_),
             static_cast<unsigned>(delivered_count));
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
      inflight_.clear();
      xSemaphoreGive(mutex_);
    }
    return true;
  }

  post_fail_.fetch_add(1, std::memory_order_relaxed);
  if (status == 400) {
    ESP_LOGE(kLogTag, "BLE API rejected invalid batch sequence=%u",
             static_cast<unsigned>(inflight_sequence_));
    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
      dropped_.fetch_add(inflight_.size(), std::memory_order_relaxed);
      inflight_.clear();
      xSemaphoreGive(mutex_);
    }
    return true;
  }
  if (status == 401 || status == 403) {
    if (config_.provider_token.isEmpty()) {
      ESP_LOGW(kLogTag,
               "Shared SK token rejected; restarting SK authorization");
      signalk_client_->restart();
    } else {
      ESP_LOGE(kLogTag,
               "Dedicated provider token rejected status=%d; check token "
               "issuer and permissions",
               status);
    }
  } else {
    ESP_LOGW(kLogTag, "POST failed result=%s status=%d sequence=%u",
             esp_err_to_name(result), status,
             static_cast<unsigned>(inflight_sequence_));
  }
  return false;
}

}  // namespace gateway
