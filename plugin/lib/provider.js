'use strict'

const { validateBatch } = require('./protocol')

const DEFAULT_BATCH_TTL_MS = 24 * 60 * 60 * 1000
const DEFAULT_MAX_BATCH_IDS = 4096
const DEFAULT_MAX_RECENT_ADVERTISEMENTS = 1000

class GatewayProvider {
  constructor(options = {}) {
    this.now = options.now || Date.now
    this.batchTtlMs = options.batchTtlMs || DEFAULT_BATCH_TTL_MS
    this.maxBatchIds = options.maxBatchIds || DEFAULT_MAX_BATCH_IDS
    this.maxRecentAdvertisements =
      options.maxRecentAdvertisements || DEFAULT_MAX_RECENT_ADVERTISEMENTS
    this.onAdvertisement = options.onAdvertisement || (() => {})
    this.seenBatches = new Map()
    this.gateways = new Map()
    this.recentAdvertisements = []
    this.receivedBatches = 0
    this.duplicateBatches = 0
    this.rejectedBatches = 0
    this.receivedAdvertisements = 0
  }

  accept(batch) {
    const validation = validateBatch(batch)
    if (!validation.valid) {
      this.rejectedBatches += 1
      return { accepted: false, status: 400, errors: validation.errors }
    }

    const now = this.now()
    this.pruneBatchIds(now)
    if (this.seenBatches.has(batch.batch_id)) {
      this.duplicateBatches += 1
      return {
        accepted: true,
        duplicate: true,
        status: 200,
        batch_id: batch.batch_id
      }
    }

    this.seenBatches.set(batch.batch_id, now)
    this.receivedBatches += 1
    this.receivedAdvertisements += batch.devices.length

    this.gateways.set(batch.gateway_id, {
      gateway_id: batch.gateway_id,
      boot_id: batch.boot_id,
      last_batch_id: batch.batch_id,
      last_sequence: batch.sequence,
      last_seen: new Date(now).toISOString(),
      uptime_ms: batch.uptime_ms,
      advertisement_count: batch.devices.length,
      firmware: typeof batch.firmware === 'string' ? batch.firmware : undefined
    })

    for (const advertisement of batch.devices) {
      const observation = {
        gateway_id: batch.gateway_id,
        boot_id: batch.boot_id,
        batch_id: batch.batch_id,
        ...advertisement
      }
      this.recentAdvertisements.push(observation)
      this.onAdvertisement({ ...observation })
    }
    const overflow = this.recentAdvertisements.length - this.maxRecentAdvertisements
    if (overflow > 0) {
      this.recentAdvertisements.splice(0, overflow)
    }

    return {
      accepted: true,
      duplicate: false,
      status: 200,
      batch_id: batch.batch_id
    }
  }

  status() {
    return {
      protocol_version: '1',
      received_batches: this.receivedBatches,
      duplicate_batches: this.duplicateBatches,
      rejected_batches: this.rejectedBatches,
      received_advertisements: this.receivedAdvertisements,
      gateways: Array.from(this.gateways.values())
    }
  }

  advertisements() {
    return this.recentAdvertisements.slice()
  }

  pruneBatchIds(now = this.now()) {
    const cutoff = now - this.batchTtlMs
    for (const [batchId, receivedAt] of this.seenBatches) {
      if (receivedAt >= cutoff && this.seenBatches.size <= this.maxBatchIds) {
        break
      }
      this.seenBatches.delete(batchId)
    }
  }
}

module.exports = { GatewayProvider }
