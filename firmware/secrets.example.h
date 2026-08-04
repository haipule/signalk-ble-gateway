#pragma once

// Copy this file to secrets.h
// and enter your own data.

#define HOSTNAME "your_hostname"
// Stable gateway identity. Do not change it with the hostname or IP address
// after commissioning.
#define GATEWAY_ID "engine-room-stbd"
#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_PASSWORD"

#define SK_SERVER "192.0.2.10"
#define SK_PORT 3000

// Bearer token for the protected gateway provider. This can be omitted when
// Signal K grants the SenseESP WebSocket client a suitable token.
#define GATEWAY_PROVIDER_TOKEN "YOUR_SIGNAL_K_BEARER_TOKEN"

#define OTA_PASSWORD "my_ota_PASSWORD"

// Local SenseESP web interface credentials. Use credentials different from
// the OTA password in production.
#define WEB_ADMIN_USERNAME "admin"
#define WEB_ADMIN_PASSWORD "my_web_PASSWORD"
