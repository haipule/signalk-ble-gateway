'use strict'

const test = require('node:test')
const assert = require('node:assert/strict')
const crypto = require('node:crypto')
const { VictronConsumer } = require('../lib/consumer')

const KEY = '00112233445566778899AABBCCDDEEFF'
const MAC = 'AA:BB:CC:DD:EE:FF'

function readout(nonce = 1, clear = Buffer.from([1, 2, 3, 4, 5, 6])) {
  const key = Buffer.from(KEY, 'hex')
  const counter = Buffer.alloc(16)
  counter.writeUInt16LE(nonce, 0)
  const cipher = crypto.createCipheriv('aes-128-ctr', key, counter)
  const header = Buffer.from([0x10, 0x00, 0x89, 0xa3, 0x02, 0, 0, key[0]])
  header.writeUInt16LE(nonce, 5)
  const payload = Buffer.concat([header, cipher.update(clear), cipher.final()])
  return { mac: MAC, rssi: -60, manufacturerData: { 737: payload.toString('hex') } }
}

const networkBeacon = { mac: MAC, rssi: -60, manufacturerData: { 737: '02BA7BE7A69A02153E16B1EA3124C39E27EB' } }

function newConsumer(clock, options) {
  return new VictronConsumer([{ id: 'house', mac: MAC, advertisementKey: KEY, enabled: true }], () => clock.t, options)
}

test('never exposes advertisement keys in status', () => {
  const key = '00112233445566778899AABBCCDDEEFF'
  const consumer = new VictronConsumer([{
    id: 'house', name: 'House battery', mac: 'aa:bb:cc:dd:ee:ff',
    advertisementKey: key, enabled: true
  }])

  const serialized = JSON.stringify(consumer.status())
  assert.equal(serialized.includes(key), false)
  assert.equal(consumer.status().devices[0].key_configured, true)
  assert.equal(consumer.status().devices[0].mac, 'AA:BB:CC:DD:EE:FF')
})

test('ignores advertisements for unconfigured devices', () => {
  const consumer = new VictronConsumer([])
  assert.equal(consumer.accept({ mac: 'AA:BB:CC:DD:EE:FF' }), null)
  assert.equal(consumer.status().ignored, 1)
})

test('non-readout beacons are ignored and keep the device online with its last reading', () => {
  const clock = { t: 1000 }
  const consumer = newConsumer(clock)
  consumer.accept(readout(1))
  clock.t += 1000
  assert.equal(consumer.accept(networkBeacon), null)
  consumer.accept(readout(2))
  clock.t += 1000
  consumer.accept(networkBeacon)

  const status = consumer.status()
  assert.equal(status.errors, 0)
  assert.equal(status.decoded, 2)
  assert.equal(status.ignored, 2)
  assert.equal(status.devices[0].online, true)
  assert.equal(status.devices[0].decode_error, null)
  assert.equal(status.devices[0].decoded.record_name, 'battery_monitor')
})

test('a decode error keeps the last good reading and does not flip online', () => {
  const clock = { t: 1000 }
  const consumer = newConsumer(clock)
  consumer.accept(readout(1))
  const good = consumer.status().devices[0].decoded
  clock.t += 1000
  const result = consumer.accept({ mac: MAC, manufacturerData: { 737: '100002' } })

  assert.match(result.state.decode_error, /too short/)
  const device = consumer.status().devices[0]
  assert.equal(consumer.status().errors, 1)
  assert.equal(device.online, true)
  assert.deepEqual(device.decoded, good)
})

test('online expires once no valid reading arrived within the timeout', () => {
  const clock = { t: 1000 }
  const consumer = newConsumer(clock, { onlineTimeoutMs: 5000 })
  consumer.accept(readout(1))
  clock.t += 4000
  consumer.accept(networkBeacon)
  assert.equal(consumer.status().devices[0].online, true)
  clock.t += 2000
  consumer.accept(networkBeacon)
  assert.equal(consumer.status().devices[0].online, false)
  consumer.accept(readout(2))
  assert.equal(consumer.status().devices[0].online, true)
})

test('a device that never decoded stays offline after errors', () => {
  const consumer = newConsumer({ t: 1000 })
  consumer.accept({ mac: MAC, manufacturerData: { 737: '100002' } })
  const device = consumer.status().devices[0]
  assert.equal(device.online, false)
  assert.equal(device.decoded, null)
})
