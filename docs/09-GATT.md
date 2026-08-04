# GATT

GATT is not part of the advertisement MVP.

In a later phase, the Signal K BLE Provider API may manage claims and provider
selection. Consumer plugins describe generic GATT operations, and the selected
provider forwards them to a gateway through a control WebSocket. Firmware may
execute connections, subscriptions, reads, and writes without understanding
payload semantics.

Manufacturer UUIDs, commands, and interpretation remain in consumer plugins.
Firmware provides generic GATT primitives only.
