# Roadmap

## Stable 0.2 release line

The 0.2 architecture and advertisement MVP are complete:

- [x] Keep firmware generic and manufacturer-independent.
- [x] Send bounded advertisement batches to Signal K's official remote BLE
  provider.
- [x] Consume the unified advertisement stream through `app.bleApi`.
- [x] Keep Victron configuration, keys, decoding, and Signal K paths in the
  server-side consumer.
- [x] Expose reset, heap, scan, queue, delivery, and categorized-drop
  diagnostics in the gateway firmware.
- [x] Complete a 90.8-hour stable-power endurance run with both gateways,
  without an observed uptime reset or scan-start failure.
- [x] Publish the stable consumer and subsequent 0.2.2 decoder expansion.

The acceptance evidence is recorded in `docs/test-record-2026-08-22.md`. The
legacy implementation remains available at `v0.1.0-rc.1` for rollback.

## Current work

### Gateway operations and health

- [ ] Confirm the current installed state of both gateways and close the
  historical shared-outage record.
- [ ] Determine the correct official BLE Provider API boundary for gateway
  liveness; do not reintroduce a parallel provider.
- [ ] Add configurable expected-gateway state, last-seen diagnostics, and
  deduplicated offline/recovery notifications.

### Integration tests

- [x] Run consumer tests automatically for pushes and pull requests.
- [ ] Add a multi-gateway integration test.
- [ ] Add repeatable outage and recovery coverage at the official provider
  boundary.

### Victron coverage

- [x] Decode and publish hardware-tested Lynx Smart BMS and Orion XS records.
- [x] Decode SmartSolar MPPT and Orion Smart DC-DC records using specification
  vectors and available real-device evidence.
- [x] Add fixture-tested decoders for the remaining published record layouts,
  while marking families without hardware validation as unverified.
- [ ] Validate the available Smart Batteries and other unverified families
  against VictronConnect before promoting their support status.
- [ ] Complete AC Charger (`0x08`) hardware validation tracked by issue #6.

### Release presentation

- [ ] Create the GitHub release for `v0.2.2`.
- [ ] Add a concise demo or screenshot and publish the updated community
  announcement.

## Later GATT and provider-selection work

- [ ] Measure resources with a separate single-device GATT prototype.
- [ ] Specify and implement the official control WebSocket contract only when
  a consumer requires GATT.
- [ ] Add claims, provider selection, and failover without coupling consumers
  to gateway transport.
- [ ] Extend on-board and automated endurance tests.
