'use strict'

const test = require('node:test')
const assert = require('node:assert/strict')
const { VictronConsumer } = require('../lib/consumer')

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
