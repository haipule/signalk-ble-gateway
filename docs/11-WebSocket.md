# Control WebSocket

A control WebSocket is reserved for a later GATT phase and is not part of the
advertisement MVP. It may carry generic gateway metadata, status events, and
GATT operations without introducing manufacturer logic into firmware.

Before implementation, message formats, versioning, authentication,
heartbeats, reconnect, correlation, timeouts, and error semantics must be
specified.

The examined upstream currently uses:

```text
ws://<server>:<port>/signalk/v2/api/ble/gateway/ws?token=<JWT>
```

It sends `hello` after connection and `status` every 30 seconds. Because the
token appears in the query string, every log path must redact it. The control
WebSocket remains disabled for protocol v1.
