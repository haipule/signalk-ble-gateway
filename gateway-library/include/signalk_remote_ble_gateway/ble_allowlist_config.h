#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include <vector>

#include "sensesp/system/saveable.h"

namespace gateway {

/**
 * Persistent, web-editable BLE MAC allowlist.
 *
 * Values are stored as normalized, comma-separated MAC addresses. An empty
 * value allows every advertiser. Changes require a restart because the
 * advertisement transport takes an immutable configuration at startup.
 */
class BleAllowlistConfig : public sensesp::FileSystemSaveable {
 public:
  BleAllowlistConfig(const String& default_value, const String& config_path);

  bool to_json(JsonObject& root) override;
  bool from_json(const JsonObject& config) override;

  const String& value() const { return value_; }
  std::vector<String> allowed_macs() const;

 private:
  static bool normalize(const String& input, String* output);

  String value_;
};

inline const String ConfigSchema(const BleAllowlistConfig&) {
  return R"({"type":"object","properties":{"value":{"title":"Allowed BLE MAC addresses","type":"string"}}})";
}

inline bool ConfigRequiresRestart(const BleAllowlistConfig&) { return true; }

}  // namespace gateway
