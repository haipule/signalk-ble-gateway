'use strict'

// Transitional in-process contract. The official Signal K BLE Provider API
// will replace only this adapter, not the Victron decoder or device model.
const ADVERTISEMENT_EVENT = 'signalk-ble-gateway:advertisement:v1'

module.exports = { ADVERTISEMENT_EVENT }
