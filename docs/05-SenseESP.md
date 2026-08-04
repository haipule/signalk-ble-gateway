# SenseESP

SenseESP v3 is a real firmware dependency. The project reuses its application,
configuration, networking, status, and OTA model instead of inventing a
parallel firmware architecture.

`dirkwa/sensesp-ble-gateway` provides the BLE abstraction and gateway
foundation. Exact revisions are pinned in `firmware/platformio.ini`.

The gateway application:

- follows SenseESP configuration and network conventions,
- encapsulates BLE interaction,
- keeps transport independent from Signal K plugins,
- remains diagnosable without a USB monitor,
- leaves manufacturer decoding to server-side consumers.

Development order is deliberately network/OTA first, then BLE scanning and
transport. This makes remote installation safer before radio functionality is
enabled.
