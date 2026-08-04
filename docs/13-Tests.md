# Test strategy

## Unit tests

- gateway-protocol validation and serialization,
- batch limits, deduplication, and bounded diagnostic storage,
- consumer filtering, decryption, and record decoding,
- provider and consumer lifecycle behavior,
- Signal K delta generation.

Hardware, network, and time dependencies should remain mockable.

## Integration tests

- firmware-compatible batches against the provider,
- authentication and protocol-version checks,
- restart, reconnect, and temporary network failure,
- repeated batches and multiple gateways,
- compatibility with the documented upstream revision,
- provider-disabled HTTP 503 behavior without consumer events,
- future migration of the provider adapter to the BLE Provider API.

## On-board tests

- headless endurance operation,
- Wi-Fi recovery and DHCP reservation,
- OTA update and restart,
- BLE scanning at the actual installation location,
- bounded memory and queue behavior during server outage,
- comparison of decoded values with VictronConnect.

GATT tests are deferred until the separate GATT phase.
