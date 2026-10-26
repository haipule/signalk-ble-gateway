# Local TODO

This file tracks work that remains after the stable 0.2 release line. Completed
release evidence is retained in `docs/test-record-2026-08-22.md` and the
changelog rather than repeated here as open work.

## Priority 1: Confirm current gateway operations

- [ ] Confirm that both installed gateways, their shared external power supply,
  and the WLAN/access point are currently available.
- [ ] Close the historical outage record for the simultaneous outage that began
  around 2026-08-15 22:35 local time. Record the cause when evidence still
  exists; otherwise mark it explicitly as unknown.

The later 90.8-hour stable-power endurance run passed for both gateways. It
does not by itself establish the cause of the earlier shared outage.

## Priority 2: Detect and notify gateway outages

- [ ] Decide, without duplicating the official provider, which Signal K BLE API
  surface owns gateway liveness and exposes the required observations.
- [ ] Detect when an expected gateway stops checking in or delivering BLE
  advertisements, with a configurable timeout and per-gateway state.
- [ ] Expose gateway health and the last-seen timestamp in diagnostics.
- [ ] Emit a Signal K notification when a gateway goes offline and when it
  recovers, without generating repeated notifications for the same outage.

## Priority 3: Complete automated integration coverage

- [x] Run the consumer test suite for pushes to `main` and pull requests.
- [ ] Add an automated integration test with multiple simulated or physical
  gateways.
- [ ] Extend automated outage, recovery, and endurance tests where the official
  provider offers a stable test surface.

## Priority 4: Expand Victron BLE coverage deliberately

- [ ] Integrate the four available Victron Smart Batteries and use them for
  local decoder validation.
- [ ] Complete hardware validation for specification-derived SmartShunt/BMV,
  Smart Battery Sense, SmartLithium, inverter, and diagnostic-only records.
- [ ] Complete AC Charger (`0x08`) validation before adding support; track the
  real-device work in GitHub issue #6.
- [ ] Keep Instant Readout advertisement support separate from connection-
  oriented GATT support; do not claim support without the required transport
  and key.
- [ ] Collect real advertisements, device keys, and VictronConnect reference
  values before declaring each parser supported.
- [ ] Keep fixture-driven tests for every implemented family and document
  unsupported or hardware-unverified families explicitly.

## Priority 5: Finish release presentation

- [ ] Create the GitHub release for `v0.2.2` and mark it as the current stable
  release.
- [ ] Improve the architecture graphic and add a short screenshot or demo of
  decoded Victron values in Signal K.
- [ ] Publish the updated announcement to the Signal K and SenseESP communities.
- [ ] Ask early users for additional device captures and test reports.

## Later work

- GATT prototype and resource measurements.
- Gateway selection, claims, and failover.
- Additional manufacturer consumers.

## Administrative cleanup

- Delete the private GitHub archive repository only after granting the GitHub
  CLI the required `delete_repo` scope and independently confirming that it no
  longer contains unique material.
