# Control WebSocket

A control WebSocket is reserved for a later GATT phase and is not part of the
advertisement MVP. It may carry generic gateway metadata, status events, and
GATT operations without introducing manufacturer logic into firmware.

Before implementation, message formats, versioning, authentication,
heartbeats, reconnect, correlation, timeouts, and error semantics must be
specified.

The official API uses:

```text
ws://<server>:<port>/signalk/v2/api/ble/gateway/ws
```

The gateway sends `hello` after connection and periodic `status` frames. Signal
K uses WebSocket ping/pong keepalive and accepts its normal authorization
sources. The control WebSocket remains disabled for the advertisement-only
0.2 beta.
