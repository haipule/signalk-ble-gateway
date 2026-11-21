'use strict'

const test = require('node:test')
const assert = require('node:assert/strict')
const { buildSchema, macPattern } = require('../lib/config-schema')
const { isShuntBatteryMonitor } = require('../lib/victron-models')

test('hides the secondary battery option until a shunt is detected', () => {
  const item = buildSchema().properties.devices.items
  assert.equal(item.properties.secondaryBattery, undefined)
  assert.equal(item.dependencies, undefined)
})

test('offers the secondary battery option only on detected shunt MACs', () => {
  const item = buildSchema(['AA:BB:CC:DD:EE:FF']).properties.devices.items
  const [shunt, other] = item.dependencies.mac.oneOf
  const pattern = new RegExp(shunt.properties.mac.pattern)

  assert.equal(shunt.properties.secondaryBattery.type, 'string')
  assert.deepEqual(other.properties.mac.not, shunt.properties.mac)
  assert.equal(other.properties.secondaryBattery, undefined)
  assert.equal(pattern.test('AA:BB:CC:DD:EE:FF'), true)
  assert.equal(pattern.test(' aa:bb:cc:dd:ee:ff '), true)
  assert.equal(pattern.test('AA:BB:CC:DD:EE:00'), false)
  assert.equal(pattern.test('AA:BB:CC:DD:EE:FF:00'), false)
})

// The admin form (RJSF with AJV) compiles patterns with the Unicode flag.
test('produces a pattern that compiles in Unicode mode', () => {
  const pattern = macPattern(['D5:9E:F7:D6:F6:92', 'A.B'])
  assert.doesNotThrow(() => new RegExp(pattern, 'u'))
  assert.equal(new RegExp(pattern, 'u').test('d5:9e:f7:d6:f6:92'), true)
  assert.equal(new RegExp(pattern, 'u').test('AxB'), false)
})

test('matches any of several MAC addresses', () => {
  const pattern = new RegExp(macPattern(['AA:BB:CC:DD:EE:FF', '11:22:33:44:55:66']))
  assert.equal(pattern.test('11:22:33:44:55:66'), true)
  assert.equal(pattern.test('aa:bb:cc:dd:ee:ff'), true)
  assert.equal(pattern.test('AA:BB:CC:DD:EE:FF|11'), false)
})

test('recognises BMV and SmartShunt models but not a Smart Battery Sense', () => {
  for (const id of [0x0203, 0xa381, 0xa389, 0xa38c, 0xc030]) {
    assert.equal(isShuntBatteryMonitor(id), true, id.toString(16))
  }
  // Smart Battery Sense, SmartShunt Bluetooth Interface, Lynx Smart BMS,
  // SmartSolar MPPT and an unknown ID.
  for (const id of [0xa3a5, 0xa196, 0xa3e5, 0xa060, 0xfffe]) {
    assert.equal(isShuntBatteryMonitor(id), false, id.toString(16))
  }
})
