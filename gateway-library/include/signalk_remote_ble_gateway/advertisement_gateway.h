#pragma once

#include <Arduino.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp_ble_gateway/ble_advertisement.h"
#include "sensesp_ble_gateway/ble_provisioner.h"
#include "signalk_remote_ble_gateway/runtime_interfaces.h"
#include "signalk_remote_ble_gateway/transport_policy.h"

namespace gateway {

struct AdvertisementGatewayConfig {
  String gateway_id;
  String gateway_mac;
  String hostname;
  String firmware_version;
  String provider_token;
  // Empty means allow all devices. Addresses are normalized before matching.
  std::vector<String> allowed_macs;
  uint32_t post_interval_ms = 2000;
  uint32_t max_retry_interval_ms = 30000;
  size_t max_pending_advertisements = 100;
  size_t max_batch_size = 20;
};

struct AdvertisementGatewayDiagnostics {
  bool running = false;
  bool scanning = false;
  bool token_available = false;
  uint32_t received = 0;
  uint32_t delivered = 0;
  uint32_t dropped = 0;
  uint32_t filtered = 0;
  uint32_t post_success = 0;
  uint32_t post_fail = 0;
  uint32_t dropped_queue_full = 0;
  uint32_t dropped_lock_timeout = 0;
  uint32_t dropped_invalid_batch = 0;
  int32_t last_http_status = 0;
  uint32_t last_post_duration_ms = 0;
  uint32_t retry_interval_ms = 0;
  uint32_t scan_start_requests = 0;
  uint32_t scan_start_failures = 0;
  size_t pending = 0;
};

/**
 * Buffers generic BLE advertisements and delivers Signal K remote-provider
 * HTTP batches.
 *
 * The transport retains its in-flight batch across transient failures. New
 * observations continue to enter a bounded pending queue. No manufacturer or
 * Signal-K path knowledge is present here.
 */
class AdvertisementGateway {
 public:
  AdvertisementGateway(
      std::shared_ptr<sensesp::BLEProvisioner> ble,
      std::shared_ptr<sensesp::SKWSClient> signalk_client,
      AdvertisementGatewayConfig config,
      std::shared_ptr<GatewayClock> clock = nullptr,
      std::shared_ptr<HttpTransport> http_transport = nullptr);
  ~AdvertisementGateway();

  AdvertisementGateway(const AdvertisementGateway&) = delete;
  AdvertisementGateway& operator=(const AdvertisementGateway&) = delete;

  bool start();
  void stop();

  AdvertisementGatewayDiagnostics diagnostics() const;

  uint32_t received() const { return received_.load(); }
  uint32_t delivered() const { return delivered_.load(); }
  uint32_t dropped() const { return dropped_.load(); }
  uint32_t filtered() const { return filtered_.load(); }
  uint32_t post_success() const { return post_success_.load(); }
  uint32_t post_fail() const { return post_fail_.load(); }
  uint32_t dropped_queue_full() const { return dropped_queue_full_.load(); }
  uint32_t dropped_lock_timeout() const {
    return dropped_lock_timeout_.load();
  }
  uint32_t dropped_invalid_batch() const {
    return dropped_invalid_batch_.load();
  }
  int32_t last_http_status() const { return last_http_status_.load(); }
  uint32_t last_post_duration_ms() const {
    return last_post_duration_ms_.load();
  }
  uint32_t retry_interval_ms() const { return retry_interval_ms_.load(); }
  uint32_t scan_start_requests() const { return scan_start_requests_.load(); }
  uint32_t scan_start_failures() const { return scan_start_failures_.load(); }
  bool running() const { return running_.load(); }
  bool token_available() const {
    return !config_.provider_token.isEmpty() ||
           (signalk_client_ && !signalk_client_->get_auth_token().isEmpty());
  }
  size_t pending() const;

 private:
  void on_advertisement();
  void task_loop();
  static void task_entry(void* argument);
  bool prepare_batch();
  bool is_allowed(const String& address) const;
  bool post_batch();
  String serialize_batch() const;

  std::shared_ptr<sensesp::BLEProvisioner> ble_;
  std::shared_ptr<sensesp::SKWSClient> signalk_client_;
  std::shared_ptr<GatewayClock> clock_;
  std::shared_ptr<HttpTransport> http_transport_;
  AdvertisementGatewayConfig config_;
  bool allowlist_configured_ = false;
  std::vector<std::string> allowed_macs_;
  BoundedBatchQueue<sensesp::BLEAdvertisement> queue_;

  mutable SemaphoreHandle_t mutex_ = nullptr;
  TaskHandle_t task_ = nullptr;
  std::atomic<bool> running_{false};

  std::atomic<uint32_t> received_{0};
  std::atomic<uint32_t> delivered_{0};
  std::atomic<uint32_t> dropped_{0};
  std::atomic<uint32_t> dropped_queue_full_{0};
  std::atomic<uint32_t> dropped_lock_timeout_{0};
  std::atomic<uint32_t> dropped_invalid_batch_{0};
  std::atomic<uint32_t> filtered_{0};
  std::atomic<uint32_t> post_success_{0};
  std::atomic<uint32_t> post_fail_{0};
  std::atomic<int32_t> last_http_status_{0};
  std::atomic<uint32_t> last_post_duration_ms_{0};
  std::atomic<uint32_t> retry_interval_ms_{0};
  std::atomic<uint32_t> scan_start_requests_{0};
  std::atomic<uint32_t> scan_start_failures_{0};
};

}  // namespace gateway
