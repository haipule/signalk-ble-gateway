# Roadmap

## 1. Architecture and protocol

- [x] Document transitional and target architecture.
- [x] Define versioned advertisement model.
- [x] Compare protocol with `dirkwa/sensesp-ble-gateway`.
- [x] Decide authentication, buffering, acknowledgement, and retry semantics.

## 2. Headless firmware foundation

- [x] Pin PlatformIO, SenseESP v3, and dependencies.
- [x] Add DHCP, hostname, authenticated web UI, and OTA.
- [x] Expose reset reason, heap, lifecycle, queue, and delivery diagnostics.
- [ ] Complete a multi-day run on stable power without an unexpected reset.

## 3. Advertisement MVP

- [x] Passive BLE scanning and bounded batching.
- [x] Authenticated HTTP POST with retained in-flight retry.
- [x] Keep GATT explicitly outside the advertisement MVP.

## 4. Gateway provider

- [x] Validate HTTP batches and expose gateway diagnostics.
- [x] Provide a stable internal consumer event.
- [x] Isolate current Signal K integration behind the provider.
- [x] Return HTTP 503 and emit no events while disabled.
- [x] Verify outage, queueing, and recovery behavior on the installed system.
- [ ] Add an integration test with multiple simulated or physical gateways.

## 5. Victron consumer

- [x] Keep device configuration and keys exclusively server-side.
- [x] Decode Lynx Smart BMS and publish battery paths.
- [x] Decode Orion XS and publish charger paths.
- [x] Provide a key-safe diagnostic web application.
- [x] Verify Orion paths and values in the Signal K data browser.

## 6. Official BLE API migration

- [x] Send firmware batches to the built-in remote BLE provider.
- [x] Move the Victron consumer to the unified advertisement stream.
- [x] Remove the transitional Signal K provider from the 0.2 branch.
- [ ] Validate against the first stable Signal K release containing PR #2588.
- [ ] Complete parallel hardware comparison with the 0.1 release.

## 7. Later GATT and multi-gateway work

- [ ] Measure resources with a separate single-device GATT prototype.
- [ ] Specify and implement a control WebSocket.
- [ ] Add claims, provider selection, and failover.
- [ ] Extend on-board and endurance tests.

## Version 0.1.0 release gate

- [x] Provider outage and recovery verified.
- [ ] At least 24 hours on stable power without reset.
- [ ] Confirm no steady-state drop increase, categorized outage losses, and
  `scan_start_failures = 0` during that run.
- [x] Compare Lynx and Orion values with VictronConnect.
- [x] Confirm Orion paths in the Signal K data browser.
- [x] Document the installed ESP32 and Signal K server state.

## 0.1.0 scope freeze

The `v0.1.0-rc.1` feature set is frozen. Only defects, tests, diagnostics, and
acceptance-documentation corrections are allowed before final release. New
manufacturer decoders, sensors, GATT, the control WebSocket, and additional
Signal K integrations belong to 0.2.0 or a separate product.

Tag `v0.1.0` only after every release-gate item is supported by evidence.
