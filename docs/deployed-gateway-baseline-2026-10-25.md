# Deployed engine-room gateway baseline

## Scope

This read-only snapshot identifies the source revisions running on the two
installed combined engine-room gateways before extracting the reusable gateway
library. No restart, flash, OTA, configuration write, or service interruption
was performed.

Observation date: 2026-10-25 local vessel time.

## Installed revisions

| Gateway | Application repository | Reported build | Matching commit | Build date |
| --- | --- | --- | --- | --- |
| Port | `signalk-engine-room-port-gateway` | `737e3a6023f6` | `737e3a6` on `wip/codex-2026-08-18` | 2026-09-12 13:13:35 |
| Starboard | `signalk-engine-room-stbd-gateway` | `399b9d77acd2` | `399b9d7` on `wip/codex-2026-08-18` | 2026-09-10 17:35:20 |

Both devices report gateway firmware `0.2.0-beta.2` and SenseESP
`3.4.1-alpha`.

## Read-only health snapshot

| Observation | Port | Starboard |
| --- | ---: | ---: |
| Uptime | 2,969,746 s | 2,970,021 s |
| Free memory | 48,668 bytes | 51,944 bytes |
| Minimum free memory | 17,936 bytes | 13,828 bytes |
| BLE scanning | yes | yes |
| Scan-start failures | 0 | 0 |
| Signal K connected | yes | yes |
| Last HTTP status | 200 | 200 |
| Pending advertisements | 0 | 10 |
| Lock-timeout drops | 0 | 0 |
| Invalid-batch drops | 0 | 0 |

Both devices reported `Software restart` as their last reset reason. Their
minimum free memory before that reset was 3,096 bytes on port and 636 bytes on
starboard. This snapshot does not establish the cause of those earlier resets.

Port allowed all observed BLE devices. Starboard used a two-address generic
MAC allowlist. The addresses themselves are intentionally omitted from this
public operational record.

## Migration consequence

The checked-out WIP branches are the deployed source baselines. Library
extraction can therefore compare local builds against exact installed commits.
Deployment remains out of scope until the standalone, port, and starboard
build gates pass and a separate flash or OTA approval and rollback plan exist.
