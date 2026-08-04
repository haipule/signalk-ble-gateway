'use strict'

const test = require('node:test')
const assert = require('node:assert/strict')
const { validateBatch } = require('../lib/protocol')
const { validBatch } = require('./helpers')

test('accepts a complete protocol v1 batch', () => {
  assert.deepEqual(validateBatch(validBatch()), { valid: true, errors: [] })
})

test('rejects unsupported versions and malformed raw data', () => {
  const batch = validBatch({
    protocol_version: '2',
    devices: [{ ...validBatch().devices[0], adv_data: 'abc' }]
  })
  const result = validateBatch(batch)
  assert.equal(result.valid, false)
  assert.match(result.errors.join('\n'), /protocol_version/)
  assert.match(result.errors.join('\n'), /adv_data/)
})

test('requires generic metadata without manufacturer-specific fields', () => {
  const batch = validBatch({
    devices: [{ ...validBatch().devices[0], mac: 'aa:bb:cc:dd:ee:ff' }]
  })
  const result = validateBatch(batch)
  assert.equal(result.valid, false)
  assert.match(result.errors.join('\n'), /uppercase colon-separated/)
})
