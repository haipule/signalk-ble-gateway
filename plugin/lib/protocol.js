'use strict'

const PROTOCOL_VERSION = '1'
const MAX_ADVERTISEMENTS_PER_BATCH = 100
const MAX_HEX_BYTES = 255
const MAX_TEXT_LENGTH = 128
const GATEWAY_ID_PATTERN = /^[A-Za-z0-9][A-Za-z0-9._:-]{0,63}$/
const MAC_PATTERN = /^(?:[0-9A-F]{2}:){5}[0-9A-F]{2}$/
const HEX_PATTERN = /^(?:[0-9A-F]{2})*$/
const ADDRESS_TYPES = new Set(['public', 'random', 'public_identity', 'random_identity', 'unknown'])

function isPlainObject(value) {
  return value !== null && typeof value === 'object' && !Array.isArray(value)
}

function isNonNegativeInteger(value) {
  return Number.isSafeInteger(value) && value >= 0
}

function validateHex(value, field, errors, required = false) {
  if (value === undefined && !required) {
    return
  }
  if (typeof value !== 'string' || !HEX_PATTERN.test(value)) {
    errors.push(`${field} must be an uppercase, even-length hexadecimal string`)
    return
  }
  if (value.length > MAX_HEX_BYTES * 2) {
    errors.push(`${field} exceeds ${MAX_HEX_BYTES} bytes`)
  }
}

function validateAdvertisement(value, index, errors) {
  const prefix = `devices[${index}]`
  if (!isPlainObject(value)) {
    errors.push(`${prefix} must be an object`)
    return
  }
  if (typeof value.mac !== 'string' || !MAC_PATTERN.test(value.mac)) {
    errors.push(`${prefix}.mac must be an uppercase colon-separated MAC address`)
  }
  if (!Number.isInteger(value.rssi) || value.rssi < -127 || value.rssi > 20) {
    errors.push(`${prefix}.rssi must be an integer from -127 through 20`)
  }
  if (!ADDRESS_TYPES.has(value.address_type)) {
    errors.push(`${prefix}.address_type is invalid`)
  }
  if (!isNonNegativeInteger(value.received_at_ms)) {
    errors.push(`${prefix}.received_at_ms must be a non-negative integer`)
  }
  if (
    value.name !== undefined &&
    (typeof value.name !== 'string' || value.name.length > MAX_TEXT_LENGTH)
  ) {
    errors.push(`${prefix}.name must be a string of at most ${MAX_TEXT_LENGTH} characters`)
  }
  validateHex(value.adv_data, `${prefix}.adv_data`, errors, true)
  validateHex(value.scan_rsp_data, `${prefix}.scan_rsp_data`, errors)
}

function validateBatch(value) {
  const errors = []
  if (!isPlainObject(value)) {
    return { valid: false, errors: ['request body must be a JSON object'] }
  }
  if (value.protocol_version !== PROTOCOL_VERSION) {
    errors.push(`protocol_version must equal "${PROTOCOL_VERSION}"`)
  }
  if (typeof value.gateway_id !== 'string' || !GATEWAY_ID_PATTERN.test(value.gateway_id)) {
    errors.push('gateway_id is invalid')
  }
  if (typeof value.boot_id !== 'string' || !GATEWAY_ID_PATTERN.test(value.boot_id)) {
    errors.push('boot_id is invalid')
  }
  if (typeof value.batch_id !== 'string' || value.batch_id.length < 1 || value.batch_id.length > 160) {
    errors.push('batch_id must contain 1 through 160 characters')
  }
  if (!isNonNegativeInteger(value.sequence)) {
    errors.push('sequence must be a non-negative integer')
  }
  if (!isNonNegativeInteger(value.uptime_ms)) {
    errors.push('uptime_ms must be a non-negative integer')
  }
  if (!Array.isArray(value.devices)) {
    errors.push('devices must be an array')
  } else {
    if (value.devices.length < 1 || value.devices.length > MAX_ADVERTISEMENTS_PER_BATCH) {
      errors.push(`devices must contain 1 through ${MAX_ADVERTISEMENTS_PER_BATCH} items`)
    }
    value.devices.forEach((device, index) => validateAdvertisement(device, index, errors))
  }
  return { valid: errors.length === 0, errors }
}

module.exports = {
  MAX_ADVERTISEMENTS_PER_BATCH,
  PROTOCOL_VERSION,
  validateBatch
}
