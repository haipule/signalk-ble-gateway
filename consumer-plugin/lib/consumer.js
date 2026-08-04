'use strict'

const { decodeEnvelope } = require('./victron')

function normalizeMac(value) {
  return String(value || '').trim().toUpperCase()
}

function publicDevice(device, state) {
  return {
    id: device.id,
    name: device.name,
    mac: normalizeMac(device.mac),
    enabled: device.enabled !== false,
    key_configured: typeof device.advertisementKey === 'string' && device.advertisementKey.length > 0,
    ...state
  }
}

class VictronConsumer {
  constructor(devices = [], now = Date.now) {
    this.now = now
    this.devices = devices.map(device => ({ ...device, mac: normalizeMac(device.mac) }))
    this.states = new Map()
    this.received = 0
    this.decoded = 0
    this.ignored = 0
    this.errors = 0
  }

  accept(advertisement) {
    this.received += 1
    const mac = normalizeMac(advertisement.mac)
    const device = this.devices.find(item => item.enabled !== false && item.mac === mac)
    if (!device) {
      this.ignored += 1
      return null
    }

    try {
      const decoded = decodeEnvelope(advertisement, device.advertisementKey)
      if (!decoded) {
        this.ignored += 1
        return null
      }
      this.decoded += 1
      const state = {
        online: true,
        last_seen: new Date(this.now()).toISOString(),
        gateway_id: advertisement.gateway_id,
        rssi: advertisement.rssi,
        decode_error: null,
        decoded
      }
      this.states.set(device.id, state)
      return { device, state }
    } catch (error) {
      this.errors += 1
      const state = {
        online: false,
        last_seen: new Date(this.now()).toISOString(),
        gateway_id: advertisement.gateway_id,
        rssi: advertisement.rssi,
        decode_error: error.message,
        decoded: null
      }
      this.states.set(device.id, state)
      return { device, state }
    }
  }

  status() {
    return {
      received: this.received,
      decoded: this.decoded,
      ignored: this.ignored,
      errors: this.errors,
      devices: this.devices.map(device => publicDevice(device, this.states.get(device.id) || {
        online: false,
        last_seen: null,
        gateway_id: null,
        rssi: null,
        decode_error: null,
        decoded: null
      }))
    }
  }
}

module.exports = { VictronConsumer, normalizeMac, publicDevice }
