# Multiple gateways

Several ESP32 gateways may observe the same BLE device. Every transport event
therefore includes a stable gateway ID. The provider receives all observations;
deduplication and aggregation happen server-side. Gateways never coordinate
with each other.

In the target architecture, the Signal K BLE Provider API owns the provider
view. A future GATT connection may select a provider by RSSI, availability, and
free connection slots. Consumers should not depend on a particular gateway ID
except for diagnostics or explicit administration.

Firmware uses DHCP. Stable IP addresses are assigned by reservations in the
managed vessel network.
