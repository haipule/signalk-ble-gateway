'use strict'

const { GatewayProvider } = require('./lib/provider')
const { ADVERTISEMENT_EVENT } = require('./lib/consumer-api')

module.exports = function createPlugin(app) {
  let running = false
  const provider = new GatewayProvider({
    onAdvertisement: advertisement => app.emit(ADVERTISEMENT_EVENT, advertisement)
  })

  const plugin = {
    id: 'signalk-ble-gateway-provider',
    name: 'Signal K BLE Gateway Provider',
    description: 'Receives generic BLE advertisements from remote gateways',

    schema: {
      type: 'object',
      properties: {}
    },

    start() {
      running = true
      app.setPluginStatus('Waiting for gateway advertisement batches')
    },

    stop() {
      running = false
      app.setPluginStatus('Stopped')
    },

    registerWithRouter(router) {
      router.post('/advertisements', (request, response) => {
        if (!running) {
          response.status(503).json({
            accepted: false,
            errors: ['Gateway provider is stopped']
          })
          return
        }

        const result = provider.accept(request.body)
        if (!result.accepted) {
          app.setPluginError(`Rejected gateway batch: ${result.errors.join('; ')}`)
          response.status(result.status).json({
            accepted: false,
            errors: result.errors
          })
          return
        }

        const status = provider.status()
        app.setPluginStatus(
          `Received ${status.received_advertisements} advertisements from ` +
            `${status.gateways.length} gateway(s)`
        )
        response.status(result.status).json(result)
      })

      router.get('/status', (_request, response) => {
        response.status(200).json({
          running,
          ...provider.status()
        })
      })

      router.get('/advertisements', (_request, response) => {
        response.status(200).json({
          protocol_version: '1',
          advertisements: provider.advertisements()
        })
      })
    }
  }

  return plugin
}
