# Reusable gateway extraction record — 2026-10-26

## Scope

Phase 2 moved the generic advertisement gateway transport from the standalone
firmware into `gateway-library/`. The standalone application consumes the
repository root as a local PlatformIO dependency. The application still owns
Bluetooth-controller reset and the creation of the BLE provisioner.

The reusable library now owns:

- generic BLE MAC normalization and allowlist filtering;
- advertisement batching and bounded pending/in-flight queues;
- HTTP delivery, retry/backoff, and scan recovery;
- transport diagnostics, including the `filtered` counter; and
- persistent allowlist parsing used by the standalone application.

No manufacturer decoder, Signal K data path, vessel identifier, or secret was
added to the library.

## Verification

The following commands were run from a clean clone on commit `c3cf976` plus
this documentation checkpoint:

```text
pio run -d firmware -e az_delivery_devkit_v4_foundation -j 8
npm test --prefix consumer-plugin
pio pkg pack .
git diff --check
```

Results:

- standalone ESP32 firmware build: success;
- PlatformIO detected `signalk-remote-ble-gateway @ 0.1.0-dev` and compiled
  both library translation units;
- firmware image: 1,888,497 of 1,966,080 bytes flash (96.1%);
- IRAM: 121,223 of 131,072 bytes (92.49%);
- DRAM sections: 62,288 bytes;
- consumer tests: 50 passed, 0 failed; and
- PlatformIO library archive creation: success.

The build emitted only existing dependency warnings. No device was flashed,
restarted, or reconfigured.

## Still open

This build proves source integration and link compatibility, not runtime
parity. Before Phase 2 can be closed, the transport needs explicit mockable
BLE, HTTP, and time seams and focused tests for filtering, queue bounds,
retries, invalid batches, and lifecycle behavior. Runtime behavior and public
protocol parity also remain unchecked until those tests and a controlled
device validation exist.

The pre-extraction standalone resource baseline was not rebuilt in this
checkpoint, so the Phase 3 resource comparison remains open.
