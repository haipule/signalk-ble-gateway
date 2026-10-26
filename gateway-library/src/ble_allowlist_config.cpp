#include "signalk_remote_ble_gateway/ble_allowlist_config.h"

#include <esp_log.h>

namespace gateway {
namespace {

constexpr char kLogTag[] = "ble_allowlist";

bool is_hex_digit(const char value) {
  return (value >= '0' && value <= '9') ||
         (value >= 'A' && value <= 'F') ||
         (value >= 'a' && value <= 'f');
}

bool is_mac_address(const String& value) {
  if (value.length() != 17) {
    return false;
  }
  for (size_t index = 0; index < value.length(); ++index) {
    if ((index + 1) % 3 == 0) {
      if (value[index] != ':') {
        return false;
      }
    } else if (!is_hex_digit(value[index])) {
      return false;
    }
  }
  return true;
}

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
    String address = input.substring(
        start, separator < 0 ? input.length() : separator);
    address.trim();
    address.replace('-', ':');
    address.toUpperCase();
    if (!is_mac_address(address)) {
      return false;
    }

    bool duplicate = false;
    int existing_start = 0;
    while (existing_start < output->length()) {
      const int existing_separator = output->indexOf(',', existing_start);
      if (output->substring(existing_start,
                            existing_separator < 0 ? output->length()
                                                   : existing_separator) ==
          address) {
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
      *output += address;
    }

    if (separator < 0) {
      break;
    }
    start = separator + 1;
  }
  return true;
}

}  // namespace gateway
