'use strict'

/**
 * Build the plugin configuration schema.
 *
 * The secondary battery option applies only to a battery monitor with an
 * auxiliary voltage input (BMV or SmartShunt). The form cannot know a
 * device's type when it is first configured: the type is only known once an
 * advertisement from that MAC has been decoded. The option is therefore shown
 * only on rows whose MAC address matches one of `secondaryBatteryMacs`, the
 * devices already detected as such a monitor.
 *
 * Hidden fields keep their saved value: the Signal K admin form does not omit
 * extra data, so a configured secondary battery survives a save made before
 * the device has been heard.
 *
 * @param {string[]} secondaryBatteryMacs normalized (upper-case) MAC addresses
 */
function buildSchema(secondaryBatteryMacs = []) {
  const item = {
    type: 'object',
    required: ['id', 'name', 'mac', 'advertisementKey'],
    properties: {
      id: { title: 'Stable device ID', type: 'string' },
      name: { title: 'Display name', type: 'string' },
      mac: { title: 'BLE MAC address', type: 'string' },
      advertisementKey: {
        title: 'Victron advertisement key',
        type: 'string',
        minLength: 32,
        maxLength: 32
      },
      enabled: { title: 'Enabled', type: 'boolean', default: true }
    }
  }

  if (secondaryBatteryMacs.length > 0) {
    const pattern = macPattern(secondaryBatteryMacs)
    item.dependencies = {
      mac: {
        oneOf: [
          {
            properties: {
              mac: { pattern },
              secondaryBattery: {
                title: 'Secondary battery ID',
                description: 'Publish the BMV/SmartShunt auxiliary voltage as a separate Signal K battery',
                type: 'string'
              }
            }
          },
          { properties: { mac: { not: { pattern } } } }
        ]
      }
    }
  }

  return {
    type: 'object',
    properties: {
      devices: {
        title: 'Victron devices',
        type: 'array',
        default: [],
        items: item
      }
    }
  }
}

/**
 * Match any of the given MAC addresses as typed in the form: surrounding
 * whitespace and either letter case, as the consumer normalizes both.
 * JSON Schema patterns take no flags, so case is spelled out per character.
 */
function macPattern(macs) {
  const alternatives = macs.map(mac => [...mac].map(character => {
    if (/[A-F]/i.test(character)) return `[${character.toLowerCase()}${character.toUpperCase()}]`
    // Escape only regex syntax: the admin form compiles patterns in Unicode
    // mode, where an identity escape such as \: is a syntax error.
    return character.replace(/[\^$\\.*+?()[\]{}|/]/g, symbol => `\\${symbol}`)
  }).join(''))
  return `^\\s*(?:${alternatives.join('|')})\\s*$`
}

module.exports = { buildSchema, macPattern }
