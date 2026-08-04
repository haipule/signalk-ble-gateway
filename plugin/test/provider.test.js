'use strict'

const test = require('node:test')
const assert = require('node:assert/strict')
const { GatewayProvider } = require('../lib/provider')
const { validBatch } = require('./helpers')

test('stores accepted advertisements and gateway status', () => {
  const provider = new GatewayProvider({ now: () => 1000 })
  const result = provider.accept(validBatch())

  assert.equal(result.accepted, true)
  assert.equal(result.duplicate, false)
  assert.equal(provider.advertisements().length, 1)
  assert.equal(provider.status().received_batches, 1)
  assert.equal(provider.status().gateways[0].gateway_id, 'engine-room-stbd')
})

test('publishes each accepted advertisement once to consumers', () => {
  const observations = []
  const provider = new GatewayProvider({
    onAdvertisement: observation => observations.push(observation)
  })
  const batch = validBatch()

  provider.accept(batch)
  provider.accept(batch)

  assert.equal(observations.length, 1)
  assert.equal(observations[0].gateway_id, batch.gateway_id)
  assert.equal(observations[0].mac, batch.devices[0].mac)
})

test('acknowledges a duplicate batch without storing it twice', () => {
  const provider = new GatewayProvider({ now: () => 1000 })
  provider.accept(validBatch())
  const duplicate = provider.accept(validBatch())

  assert.equal(duplicate.accepted, true)
  assert.equal(duplicate.duplicate, true)
  assert.equal(provider.advertisements().length, 1)
  assert.equal(provider.status().duplicate_batches, 1)
})

test('rejects invalid batches and counts the rejection', () => {
  const provider = new GatewayProvider()
  const result = provider.accept({})

  assert.equal(result.accepted, false)
  assert.equal(result.status, 400)
  assert.equal(provider.status().rejected_batches, 1)
})

test('bounds the recent advertisement buffer', () => {
  const provider = new GatewayProvider({ maxRecentAdvertisements: 1 })
  provider.accept(validBatch())
  provider.accept(
    validBatch({
      batch_id: 'engine-room-stbd:a1b2c3d4:43',
      sequence: 43,
      devices: [{ ...validBatch().devices[0], mac: '11:22:33:44:55:66' }]
    })
  )

  assert.equal(provider.advertisements().length, 1)
  assert.equal(provider.advertisements()[0].mac, '11:22:33:44:55:66')
})
