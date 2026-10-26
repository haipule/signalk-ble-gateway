# Local TODO

This file tracks work that remains after the stable 0.2 release line. Check an
item only when the repository, build output, test result, or operational record
contains the corresponding evidence. At every interruption, update the active
checkpoint before starting unrelated work.

## Active checkpoint: reusable gateway library

- Status: deployed baseline confirmed; implementation not started.
- Last reviewed: 2026-10-25.
- Source of truth: this `signalk-ble-gateway` repository.
- Integration consumers:
  - `/home/hans/projects/signalk-engine-room-port-gateway`
  - `/home/hans/projects/signalk-engine-room-stbd-gateway`
- Next exact action: write the library-boundary ADR, then run the local
  PlatformIO packaging spike without changing either integration repository.
- Current blocker: none for ADR, packaging, tests, or local builds. Onboard
  deployment remains separately gated.
- Safety boundary: extraction, tests, and local builds are allowed. Do not
  flash, perform OTA, restart a gateway, or change the installed configuration
  without a separate explicit approval and rollback plan.

## Priority 1: Extract the reusable gateway library

### Phase 0: Decisions and deployed-state evidence

- [x] Identify and inspect the combined port and starboard engine-room firmware
  repositories, including their project rules and hardware documentation.
- [x] Confirm that both onboard variants currently contain identical copied
  gateway transport and allowlist implementations.
- [x] Select this repository as the single source of truth for generic gateway
  transport.
- [x] Keep the library in this repository initially and perform a packaging
  spike before deciding whether a separate repository is justified.
- [x] Keep application `setup()`, local sensors, GPIO assignments, Signal K
  sensor paths, web/OTA setup, and secrets outside the library.
- [x] Make the application, not the library, responsible for the global
  Bluetooth-controller lifecycle.
- [x] Require onboard builds to pin an immutable gateway tag or exact commit;
  never consume a moving `main` branch in production.
- [x] Keep the first phase build-only; require separate approval before any
  flash or OTA action.
- [x] Read the running firmware build identity from the port gateway and map it
  to an exact repository commit.
- [x] Read the running firmware build identity from the starboard gateway and
  map it to an exact repository commit.
- [x] Record whether the installed firmware matches the respective
  `wip/codex-2026-08-18` branch or another revision.

Evidence: `docs/deployed-gateway-baseline-2026-10-25.md`.

### Phase 1: Package and interface design

- [ ] Add an ADR defining library ownership, dependency direction, Bluetooth
  lifecycle, versioning, and the migration boundary.
- [ ] Choose a distinct package name that cannot be confused with
  `dirkwa/sensesp-ble-gateway`; preferred working name:
  `signalk-remote-ble-gateway`.
- [ ] Prove with a minimal PlatformIO packaging spike that this repository can
  be consumed through `lib_deps` while retaining the npm consumer and
  standalone firmware.
- [ ] Define the smallest public configuration and diagnostics interfaces.
- [ ] Define separate tag namespaces for the npm consumer and gateway library
  so a library tag cannot trigger npm publication.
- [ ] Document the dependency chain: upstream BLE abstraction -> remote gateway
  library -> standalone or onboard application.

### Phase 2: Extract without changing behavior

- [ ] Bring the generic MAC allowlist, MAC normalization, and `filtered`
  diagnostics from the onboard variants into the source-of-truth implementation.
- [ ] Move advertisement batching, bounded queues, retry/backoff, HTTP
  transport, scan recovery, allowlist filtering, and diagnostics into the
  reusable library.
- [ ] Keep Bluetooth controller reset/normalization as an explicit application
  action; the library must not silently deinitialize global Bluetooth state.
- [ ] Convert the standalone firmware to consume the local library.
- [ ] Add tests for configuration, filtering, queue bounds, retries, invalid
  batches, and lifecycle behavior using mockable BLE, network, and time seams.
- [ ] Confirm that the standalone gateway behavior and public protocol remain
  unchanged.

### Phase 3: Build and resource gates

- [ ] Build the standalone firmware and record flash, IRAM, static RAM, and
  relevant runtime diagnostics against the pre-extraction baseline.
- [ ] Convert the port integration in a reviewable branch to an exact library
  commit; remove its copied gateway files only after a successful build.
- [ ] Build the complete port firmware and record flash, IRAM, static RAM, and
  remaining OTA partition space. Baseline: 1,923,385 of 1,966,080 bytes flash
  and 122,551 of 131,072 bytes IRAM.
- [ ] Convert the starboard integration in a reviewable branch to the same exact
  library commit; remove its copied gateway files only after a successful build.
- [ ] Build the complete starboard firmware and record flash, IRAM, static RAM,
  and remaining OTA partition space. Baseline: 1,925,745 of 1,966,080 bytes
  flash and 122,551 of 131,072 bytes IRAM.
- [ ] Expose application firmware version, gateway library version, and Git
  build identity separately in diagnostics.
- [ ] Confirm that no secrets, vessel-specific identifiers, manufacturer
  decoding, or local Signal K sensor paths entered the library.

### Phase 4: Controlled onboard migration

- [ ] Select the first migration gateway; port is preferred provisionally
  because it has no engine-state digital inputs.
- [ ] Prepare exact backup, USB recovery image, previous dependency pin, and
  rollback instructions for the selected gateway.
- [ ] Obtain explicit approval for the first flash or OTA deployment.
- [ ] Verify after deployment: local sensors, Signal K paths, BLE scanning,
  delivery counters, queue recovery, web UI, OTA readiness, heap, and reset
  reason.
- [ ] Complete a bounded observation period on the first gateway before
  changing the second gateway.
- [ ] Repeat backup, approval, deployment, verification, and observation for the
  second gateway.
- [ ] Tag the first released gateway-library version only after both integration
  builds and the approved hardware validation have passed.

## Priority 2: Confirm current gateway operations

- [ ] Confirm that both installed gateways, their shared external power supply,
  and the WLAN/access point are currently available.
- [ ] Close the historical outage record for the simultaneous outage that began
  around 2026-08-15 22:35 local time. Record the cause when evidence still
  exists; otherwise mark it explicitly as unknown.

The later 90.8-hour stable-power endurance run passed for both gateways. It
does not by itself establish the cause of the earlier shared outage.

## Priority 3: Detect and notify gateway outages

- [ ] Decide, without duplicating the official provider, which Signal K BLE API
  surface owns gateway liveness and exposes the required observations.
- [ ] Detect when an expected gateway stops checking in or delivering BLE
  advertisements, with a configurable timeout and per-gateway state.
- [ ] Expose gateway health and the last-seen timestamp in diagnostics.
- [ ] Emit a Signal K notification when a gateway goes offline and when it
  recovers, without generating repeated notifications for the same outage.

## Priority 4: Complete automated integration coverage

- [x] Run the consumer test suite for pushes to `main` and pull requests.
- [ ] Add an automated integration test with multiple simulated or physical
  gateways.
- [ ] Extend automated outage, recovery, and endurance tests where the official
  provider offers a stable test surface.

## Priority 5: Expand Victron BLE coverage deliberately

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

## Priority 6: Finish release presentation

- [x] Create the GitHub release for `v0.2.2` and mark it as the current stable
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
