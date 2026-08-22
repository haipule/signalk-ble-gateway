# BLE gateway endurance acceptance record

## Result

The operational endurance test passed. Both gateways remained usable for the
complete observation window. The formal drop-cause breakdown is limited because
the recorded schema did not contain the optional `drop_queue`, `drop_lock`, and
`drop_invalid` counters.

## Observation

- Host: Raspberry Pi `openplotter`
- Observation window: `2026-08-19T01:25:33Z` to `2026-08-22T20:15:50Z`
- Duration: approximately 90.8 hours
- Samples: 5,364 JSONL records (schema version 2)
- Host boot identity remained constant
- `ble-endurance.timer` was active during the test and was disabled afterward
- The active log directory was removed after archiving
- Acceptance archive: `/home/pi/ble-endurance-acceptance-20260822-161816.tar.gz`

## Gateway observations

Both gateways remained in BLE scanning state with zero scan-start failures.
There was one unreachable backbord sample; no gateway uptime reset was observed.
The three older logger gaps were approximately 120, 129, and 125 seconds. The
two gaps on 22 August (approximately seven minutes each) were intentional and
are accepted as planned interruptions.

Counter changes over the observation window:

| Gateway | Received delta | Delivered delta | Dropped delta | Final pending |
| --- | ---: | ---: | ---: | ---: |
| Steuerbord | 2,217,532 | 1,454,147 | 4,485 | 3 |
| Backbord | 1,976,101 | 769,584 | 1,992 | 2 |

Free heap fluctuated without an obvious downward trend. The recorded reset
reason was `Software restart`; `firmware_build` was not populated. These are
documentation limitations, not evidence of an unexpected reset during this
observation window.

## Cleanup

After the observation, `ble-endurance.timer` and the service were stopped; the
timer was disabled. The JSONL files were retained in the compressed acceptance
archive on the Raspberry Pi.

