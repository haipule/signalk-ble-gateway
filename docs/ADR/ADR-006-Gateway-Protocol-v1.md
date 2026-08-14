# ADR-006: Gateway protocol v1 advertisement batches

## Status

Superseded for 0.2 by Signal K PR #2588. Normative only for the 0.1 rollback
release line.

## Context

The BLE Provider API proposed in Signal K PR #2588 is not part of a stable
release. The reference library posts directly to its provisional endpoint,
drops failed batches, and uses hostname as gateway identity. Firmware needs a
stable transport independent of Signal K internals.

## Decision

Protocol v1 uses batched HTTP POST requests to this project's gateway provider.

- The transitional provider is a normal Signal K plugin.
- HTTP is initially allowed only on a managed, trusted vessel network.
- The endpoint temporarily uses a Signal K read-write bearer token.
- `gateway_id` is stable configuration, independent of hostname.
- Each boot creates `boot_id`; gateway, boot, and sequence form `batch_id`.
- Firmware retains an in-flight batch until a 2xx response and retries
  transient failure with bounded exponential backoff.
- The provider deduplicates repeated batch IDs.
- All buffers are bounded. Persistent overflow drops the oldest observations
  not yet assigned to an in-flight batch and increments a visible counter.
- Complete advertising payload, address type, boot-relative receive time, and
  optional scan response are preserved.
- GATT and the control WebSocket are not protocol v1.

## Security boundary

Bearer-token HTTP is acceptable only on an administered trusted network.
Tokens must never enter logs or diagnostics. Untrusted networks require HTTPS.
The reused Signal K credential is transitional and can later be replaced
without changing the advertisement model.

## Migration

The merged API includes its own remote provider, so 0.2 changes the firmware
HTTP contract and removes the transitional plugin instead of translating
through it.
