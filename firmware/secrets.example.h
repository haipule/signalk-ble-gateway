#pragma once

// Copy this file to secrets.h
// and enter your own data.

#define HOSTNAME "your_hostname"
// Stable identity used by Signal K's built-in remote BLE provider.
#define GATEWAY_ID "engine-room-stbd"
#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_PASSWORD"

#define SK_SERVER "192.0.2.10"
#define SK_PORT 3000

// Bearer token accepted by Signal K's official remote BLE gateway endpoint.
// This can be omitted when the SenseESP WebSocket client has a suitable token.
#define GATEWAY_PROVIDER_TOKEN "YOUR_SIGNAL_K_BEARER_TOKEN"

// Initial comma-separated BLE MAC allowlist. Empty means receive all. After
// first boot the value is managed through the authenticated SenseESP UI.
#define GATEWAY_BLE_MAC_ALLOWLIST ""

#define OTA_PASSWORD "my_ota_PASSWORD"

// Local SenseESP web interface credentials. Use credentials different from
// the OTA password in production.
#define WEB_ADMIN_USERNAME "admin"
#define WEB_ADMIN_PASSWORD "my_web_PASSWORD"
