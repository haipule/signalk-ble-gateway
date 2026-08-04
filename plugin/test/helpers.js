'use strict'

function validBatch(overrides = {}) {
  return {
    protocol_version: '1',
    gateway_id: 'engine-room-stbd',
    boot_id: 'a1b2c3d4',
    batch_id: 'engine-room-stbd:a1b2c3d4:42',
    sequence: 42,
    uptime_ms: 123456,
    firmware: '0.2.0',
    devices: [
      {
        mac: 'AA:BB:CC:DD:EE:FF',
        address_type: 'random',
        rssi: -61,
        received_at_ms: 123400,
        name: 'sensor',
        adv_data: '020106',
        scan_rsp_data: ''
      }
    ],
    ...overrides
  }
}

module.exports = { validBatch }
