'use strict'

function parseHex(value) {
  if (typeof value !== 'string' || value.length % 2 !== 0 || !/^[0-9A-Fa-f]*$/.test(value)) {
    throw new Error('invalid advertisement hex')
  }
  return Buffer.from(value, 'hex')
}

function parseAdStructures(value) {
  const bytes = parseHex(value)
  const structures = []
  for (let offset = 0; offset < bytes.length;) {
    const length = bytes[offset]
    if (length === 0) break
    const end = offset + length + 1
    if (end > bytes.length || length < 1) throw new Error('truncated AD structure')
    structures.push({ type: bytes[offset + 1], data: bytes.subarray(offset + 2, end) })
    offset = end
  }
  return structures
}

function manufacturerData(value, companyId) {
  for (const structure of parseAdStructures(value)) {
    if (structure.type !== 0xff || structure.data.length < 2) continue
    if (structure.data.readUInt16LE(0) === companyId) return structure.data.subarray(2)
  }
  return null
}

module.exports = { manufacturerData, parseAdStructures, parseHex }
