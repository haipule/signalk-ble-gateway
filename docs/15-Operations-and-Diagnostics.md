# Operations and diagnostics

The gateway must remain diagnosable and updateable in an engine-room
installation without a permanent USB monitor.

## Install Signal K plugins

On the Signal K host:

```sh
cd /path/to/signalk-ble-gateway/plugin
npm link
cd ~/.signalk
npm link signalk-ble-gateway-provider

cd /path/to/signalk-ble-gateway/consumer-plugin
npm link
cd ~/.signalk
npm link signalk-victron-ble-consumer
```

Restart Signal K. Enable the gateway provider first, then the Victron consumer.
Configure every consumer device with a stable ID, display name, BLE MAC, and
32-character advertisement key. Never copy keys into firmware, logs, issues,
or diagnostic exports.

## Configure and flash the ESP32

```sh
cp firmware/secrets.example.h firmware/include/secrets.h
cd firmware
pio run -e az_delivery_devkit_v4_foundation
pio run -e az_delivery_devkit_v4_foundation -t upload
pio device monitor
```

Fill the local header with Wi-Fi, Signal K address, stable gateway ID, and
separate OTA and web passwords. Perform the first flash over USB. Use OTA only
after board, network, and credentials have been verified.

## Commissioning checklist

1. Enable provider and consumer.
2. Power the ESP32 from a stable supply.
3. Confirm `scanning`, `token=available`, and `start_failures=0` on status.
4. Open `/plugins/signalk-ble-gateway-provider/status`.
5. Open `/signalk-victron-ble-consumer/`.
6. Compare Lynx and Orion values with simultaneous VictronConnect readings.
7. Confirm Orion paths in the Signal K data browser.

Protected routes require an authenticated browser session or bearer token.
Read tokens invisibly in terminal scripts; never place them in shell history.

## Diagnostic layers

### Serial log

Development and first commissioning use ESP-IDF logs over USB/UART. Production
logs report state transitions, warnings, and errors, but not full advertisement
payloads or credentials.

### Authenticated SenseESP status page

The local UI reports firmware/build ID, hostname, uptime, reset reason, current
and minimum heap, Wi-Fi status/RSSI, BLE scan state, token availability,
advertisement counters, queue depth, drops, and POST outcomes.

### Provider and consumer status

- `/plugins/signalk-ble-gateway-provider/status` reports provider lifecycle,
  gateways, accepted batches, duplicates, and recent observations.
- `/plugins/signalk-victron-ble-consumer/status` reports configured devices,
  online state, last observation, decode errors, and decoded diagnostics.

The transport prefers `GATEWAY_PROVIDER_TOKEN` from ignored `secrets.h`; it can
fall back to the SenseESP Signal K token. Token values are never logged.

## OTA

```sh
GATEWAY_OTA_HOST=192.0.2.31 \
GATEWAY_OTA_PASSWORD='...' \
pio run -e az_delivery_devkit_v4_foundation_ota -t upload
```

Test updates on an accessible device first. A failed update must not remove the
physical USB recovery path.

## Recovery behavior

Transient failures use bounded retry/backoff, network reconnection, component
reinitialization, and only then a controlled restart. An offline server must
not create a reboot loop. Every automatic reset needs a visible reason.

## 0.1.0 endurance acceptance

Run at least 24 hours on stable power. At completion:

- uptime covers the complete observation period,
- no brownout or unexpected reset occurred,
- BLE remains `scanning`,
- `scan_start_failures = 0`,
- `dropped = 0`,
- `pending` repeatedly returns to zero or a small value,
- `delivered` continues to follow `received`,
- minimum heap does not trend downward.

A shorter uptime or `Brownout` reset reason fails this test.

## Provider outage test

1. Record gateway and provider counters.
2. Disable the gateway provider.
3. Confirm status reports `running: false` and POST returns HTTP 503.
4. Confirm consumer timestamps stop while the firmware queue and POST-failure
   counters respond as designed.
5. Re-enable the provider.
6. Confirm `running: true`, queue recovery, and resumed delivery.
7. Confirm repeated batch IDs never duplicate consumer events.

## Acceptance record

Record date, firmware version, plugin commit, gateway ID, supply, duration,
reset reason, minimum heap, and final counters. Track deviations as roadmap
items rather than replacing evidence with a generic "works" statement.
