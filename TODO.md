# Local TODO

This file is a local working note. Do not commit or publish it yet.

## Priority 1: Restore both gateways

- Check the shared external power supply and WLAN/access point.
- Record the cause and duration of the simultaneous outage that began around
  2026-08-15 22:35 local time.
- Avoid reflashing before the reset reason and surrounding evidence have been
  collected where possible.

## Priority 2: Complete endurance testing

- Restore or restart the temporary Raspberry Pi logger.
- Run at least 24 hours, preferably 72 hours, on stable external power.
- Record uptime, reset reason, free/minimum heap, Wi-Fi RSSI, BLE scan state,
  Signal K connection, provider count, device observations, queue state, and
  delivery counters.
- Confirm automatic recovery after temporary WLAN or Signal K interruption.

## Priority 3: Investigate dropped advertisements

- [x] Explain the dominant non-zero `dropped` counters: 93--97% of new drops
  accumulated during the shared 92-minute outage and bounded-queue overflow.
- [x] Measure the increase and correlate it with queue pressure. Remaining
  short spikes affect the higher-rate starboard gateway more strongly.
- [x] Remove the artificial one-batch-per-two-seconds backlog limit while
  retaining bounded memory.
- [x] Split queue, lock-timeout, and invalid-batch drop diagnostics.
- [x] Define zero drops for steady connected operation; long intentional
  outages must instead have categorized, bounded losses and clean recovery.

## Priority 4: Finish outage and recovery acceptance

- Verify that `pending` returns to zero or a small value after recovery.
- Verify that delivery resumes without incorrect or duplicated Signal K state.
- Confirm `scan_start_failures = 0` and no unexpected restart or brownout.
- Save a final acceptance record for each gateway.

## Priority 5: Stabilize the 0.2 release

- Validate the complete setup against Signal K Server 2.31.
- Add an automated multi-gateway integration test.
- Reconcile the roadmap and release-gate wording with the 0.2 version line.

## Priority 6: Promote the consumer from beta

- Publish `signalk-victron-ble-consumer@0.2.0` after endurance acceptance.
- Add the stable GitHub release and final changelog entry.
- Point npm `latest` deliberately at the stable version.

## Priority 7: Announce the project

- Improve the short installation path, architecture graphic, and screenshots.
- Announce it to the Signal K and SenseESP communities.
- Present the gateway as generic BLE transport and the Victron decoder as a
  separate server-side consumer.

## Later work

- GATT prototype and resource measurements.
- Gateway selection, claims, and failover.
- Additional manufacturer consumers.
- Extended automated endurance testing.

## Administrative cleanup

- Delete the private GitHub archive repository after granting the GitHub CLI
  the required `delete_repo` scope.
