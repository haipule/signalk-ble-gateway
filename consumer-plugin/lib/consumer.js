'use strict'

const { decodeEnvelope } = require('./victron')

// A device counts as online while it keeps producing valid readings. Victron
// devices advertise about once per second, interleaved with other beacons.
const DEFAULT_ONLINE_TIMEOUT_MS = 120000

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
  constructor(devices = [], now = Date.now, { onlineTimeoutMs = DEFAULT_ONLINE_TIMEOUT_MS } = {}) {
    this.now = now
    this.onlineTimeoutMs = onlineTimeoutMs
    this.devices = devices.map(device => ({ ...device, mac: normalizeMac(device.mac) }))
    this.states = new Map()
    this.decodedAt = new Map()
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
      this.decodedAt.set(device.id, this.now())
      this.states.set(device.id, state)
      return { device, state }
    } catch (error) {
      this.errors += 1
      // Keep the last good reading: one bad packet must not erase it.
      const previous = this.states.get(device.id)
      const state = {
        online: this.isOnline(device.id),
        last_seen: new Date(this.now()).toISOString(),
        gateway_id: advertisement.gateway_id,
        rssi: advertisement.rssi,
        decode_error: error.message,
        decoded: previous?.decoded || null
      }
      this.states.set(device.id, state)
      return { device, state }
    }
  }

  isOnline(id) {
    const decodedAt = this.decodedAt.get(id)
    return decodedAt !== undefined && this.now() - decodedAt <= this.onlineTimeoutMs
  }

  status() {
    return {
      received: this.received,
      decoded: this.decoded,
      ignored: this.ignored,
      errors: this.errors,
      devices: this.devices.map(device => publicDevice(device, this.states.has(device.id) ? {
        ...this.states.get(device.id),
        online: this.isOnline(device.id)
      } : {
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
