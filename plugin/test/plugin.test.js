'use strict'

const test = require('node:test')
const assert = require('node:assert/strict')
const createPlugin = require('../index')
const { validBatch } = require('./helpers')

function responseRecorder() {
  return {
    statusCode: undefined,
    body: undefined,
    status(code) {
      this.statusCode = code
      return this
    },
    json(body) {
      this.body = body
      return this
    }
  }
}

function pluginHarness() {
  const routes = new Map()
  const statuses = []
  const errors = []
  const events = []
  const app = {
    setPluginStatus: (status) => statuses.push(status),
    setPluginError: (error) => errors.push(error),
    emit: (name, value) => events.push({ name, value })
  }
  const router = {
    post: (path, handler) => routes.set(`POST ${path}`, handler),
    get: (path, handler) => routes.set(`GET ${path}`, handler)
  }
  const plugin = createPlugin(app)
  plugin.registerWithRouter(router)
  return { errors, events, plugin, routes, statuses }
}

test('registers readwrite ingestion and readonly diagnostic routes', () => {
  const harness = pluginHarness()
  assert.deepEqual(harness.plugin.schema, {
    type: 'object',
    properties: {}
  })
  assert.equal(harness.routes.has('POST /advertisements'), true)
  assert.equal(harness.routes.has('GET /status'), true)
  assert.equal(harness.routes.has('GET /advertisements'), true)
})

test('accepts a valid batch through the HTTP handler', () => {
  const harness = pluginHarness()
  const response = responseRecorder()

  harness.plugin.start()
  harness.routes.get('POST /advertisements')({ body: validBatch() }, response)

  assert.equal(response.statusCode, 200)
  assert.equal(response.body.accepted, true)
  assert.match(harness.statuses.at(-1), /Received 1 advertisements/)
  assert.equal(harness.events.length, 1)
})

test('returns HTTP 400 and a useful error for an invalid batch', () => {
  const harness = pluginHarness()
  const response = responseRecorder()

  harness.plugin.start()
  harness.routes.get('POST /advertisements')({ body: {} }, response)

  assert.equal(response.statusCode, 400)
  assert.equal(response.body.accepted, false)
  assert.equal(harness.errors.length, 1)
})

test('rejects batches without emitting events while stopped', () => {
  const harness = pluginHarness()
  const responseBeforeStart = responseRecorder()

  harness.routes.get('POST /advertisements')(
    { body: validBatch() },
    responseBeforeStart
  )

  assert.equal(responseBeforeStart.statusCode, 503)
  assert.deepEqual(responseBeforeStart.body, {
    accepted: false,
    errors: ['Gateway provider is stopped']
  })
  assert.equal(harness.events.length, 0)

  harness.plugin.start()
  harness.plugin.stop()
  const responseAfterStop = responseRecorder()
  harness.routes.get('POST /advertisements')(
    { body: validBatch() },
    responseAfterStop
  )

  assert.equal(responseAfterStop.statusCode, 503)
  assert.equal(harness.events.length, 0)
})

test('reports whether the provider is running', () => {
  const harness = pluginHarness()
  const stoppedResponse = responseRecorder()
  harness.routes.get('GET /status')({}, stoppedResponse)
  assert.equal(stoppedResponse.body.running, false)

  harness.plugin.start()
  const runningResponse = responseRecorder()
  harness.routes.get('GET /status')({}, runningResponse)
  assert.equal(runningResponse.body.running, true)
})
