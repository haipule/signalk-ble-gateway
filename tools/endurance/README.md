# BLE endurance logger

The logger records one structured JSON object per minute without storing
credentials, device addresses, Wi-Fi names, or advertisement keys.

Configure the installation in `/etc/default/ble-endurance`:

```sh
BLE_GATEWAY_STBD_URL=http://192.0.2.31
BLE_GATEWAY_PORT_URL=http://192.0.2.32
SIGNALK_URL=http://127.0.0.1:3000
BLE_ENDURANCE_OUTPUT_DIR=/home/pi/signalk-ble-endurance
```

The systemd service should load that file with:

```ini
EnvironmentFile=/etc/default/ble-endurance
```

Schema version 2 records parsed BLE counters, individual drop reasons, HTTP
status and latency reported by new firmware, gateway request duration, Signal K
API latency, firmware build identity, host boot identity, and service restart
state. Older firmware remains compatible; unavailable fields are simply absent.

Keep old JSONL data as a separate acceptance record before changing firmware or
logger schema.
