#include "signalk_remote_ble_gateway/ble_allowlist_config.h"

#include "signalk_remote_ble_gateway/transport_policy.h"

#include <esp_log.h>

namespace gateway {
namespace {

constexpr char kLogTag[] = "ble_allowlist";

}  // namespace

BleAllowlistConfig::BleAllowlistConfig(const String& default_value,
                                       const String& config_path)
    : FileSystemSaveable(config_path) {
  if (!normalize(default_value, &value_)) {
    ESP_LOGE(kLogTag, "Invalid compiled BLE MAC allowlist; allowing all");
    value_ = "";
  }
  load();
}

bool BleAllowlistConfig::to_json(JsonObject& root) {
  root["value"] = value_;
  return true;
}

bool BleAllowlistConfig::from_json(const JsonObject& config) {
  if (!config["value"].is<String>()) {
    return false;
  }

  String normalized;
  if (!normalize(config["value"].as<String>(), &normalized)) {
    return false;
  }
  value_ = normalized;
  return true;
}

std::vector<String> BleAllowlistConfig::allowed_macs() const {
  std::vector<String> result;
  int start = 0;
  while (start < value_.length()) {
    const int separator = value_.indexOf(',', start);
    result.push_back(value_.substring(
        start, separator < 0 ? value_.length() : separator));
    if (separator < 0) {
      break;
    }
    start = separator + 1;
  }
  return result;
}

bool BleAllowlistConfig::normalize(const String& input, String* output) {
  output->clear();
  int start = 0;
  while (start < input.length()) {
    const int separator = input.indexOf(',', start);
    const String address = input.substring(
        start, separator < 0 ? input.length() : separator);
    std::string normalized_address;
    if (!normalize_mac_address(address.c_str(), &normalized_address)) {
      return false;
    }
    const String normalized = normalized_address.c_str();

    bool duplicate = false;
    int existing_start = 0;
    while (existing_start < output->length()) {
      const int existing_separator = output->indexOf(',', existing_start);
      if (output->substring(existing_start,
                            existing_separator < 0 ? output->length()
                                                   : existing_separator) ==
          normalized) {
        duplicate = true;
        break;
      }
      if (existing_separator < 0) {
        break;
      }
      existing_start = existing_separator + 1;
    }
    if (!duplicate) {
      if (!output->isEmpty()) {
        *output += ',';
      }
      *output += normalized;
    }

    if (separator < 0) {
      break;
    }
    start = separator + 1;
  }
  return true;
}

}  // namespace gateway
