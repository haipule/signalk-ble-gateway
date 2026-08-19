#pragma once

#include <Arduino.h>

#include <atomic>
#include <memory>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sensesp/signalk/signalk_ws_client.h"
#include "sensesp_ble_gateway/ble_advertisement.h"
#include "sensesp_ble_gateway/ble_provisioner.h"

namespace gateway {

struct AdvertisementGatewayConfig {
  String gateway_id;
  String gateway_mac;
  String hostname;
  String firmware_version;
  String provider_token;
  uint32_t post_interval_ms = 2000;
  uint32_t max_retry_interval_ms = 30000;
  size_t max_pending_advertisements = 100;
  size_t max_batch_size = 20;
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
      AdvertisementGatewayConfig config);
  ~AdvertisementGateway();

  AdvertisementGateway(const AdvertisementGateway&) = delete;
  AdvertisementGateway& operator=(const AdvertisementGateway&) = delete;

  bool start();
  void stop();

  uint32_t received() const { return received_.load(); }
  uint32_t delivered() const { return delivered_.load(); }
  uint32_t dropped() const { return dropped_.load(); }
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
  bool post_batch();
  String serialize_batch() const;

  std::shared_ptr<sensesp::BLEProvisioner> ble_;
  std::shared_ptr<sensesp::SKWSClient> signalk_client_;
  AdvertisementGatewayConfig config_;
  uint32_t next_sequence_ = 0;
  uint32_t inflight_sequence_ = 0;

  mutable SemaphoreHandle_t mutex_ = nullptr;
  std::vector<sensesp::BLEAdvertisement> pending_;
  std::vector<sensesp::BLEAdvertisement> inflight_;
  TaskHandle_t task_ = nullptr;
  std::atomic<bool> running_{false};

  std::atomic<uint32_t> received_{0};
  std::atomic<uint32_t> delivered_{0};
  std::atomic<uint32_t> dropped_{0};
  std::atomic<uint32_t> dropped_queue_full_{0};
  std::atomic<uint32_t> dropped_lock_timeout_{0};
  std::atomic<uint32_t> dropped_invalid_batch_{0};
  std::atomic<uint32_t> post_success_{0};
  std::atomic<uint32_t> post_fail_{0};
  std::atomic<int32_t> last_http_status_{0};
  std::atomic<uint32_t> last_post_duration_ms_{0};
  std::atomic<uint32_t> retry_interval_ms_{0};
  std::atomic<uint32_t> scan_start_requests_{0};
  std::atomic<uint32_t> scan_start_failures_{0};
};

}  // namespace gateway
