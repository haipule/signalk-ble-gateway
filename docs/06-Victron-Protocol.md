# Victron BLE protocol

Victron advertisements are decrypted and decoded exclusively by a server-side
consumer. The ESP32 forwards device address, generic metadata, and raw payload;
it does not know Victron devices or keys.

Device MAC addresses and advertisement keys belong in consumer configuration.
They must never appear in firmware headers, binaries, or gateway-provider
configuration. Passive Instant Readout advertisements require no GATT session.

## Instant Readout envelope

The consumer extracts manufacturer data with company ID `0x02E1`, followed by
the product-advertisement header `0x10 0x00`, 16-bit model ID, record type,
16-bit nonce/data counter, the first advertisement-key byte as a quick check,
and the AES-128-CTR encrypted record.

The counter is written little-endian into the 128-bit CTR block. Record layouts
are implemented only with reproducible test vectors before values are mapped to
Signal K.

The published Victron specification names Lynx fields `VE_REG_BMS_IO` and
`VE_REG_BMS_WARNINGS_ALARMS` but does not document their internal bit layout.
The web application therefore displays both as raw hexadecimal values and does
not invent bit meanings.

## Orion XS

The observed Orion XS model `0xA3F8` uses record type `0x0F`. Its published
layout includes operating state, error code, output voltage/current, input
voltage/current, and a 32-bit shutdown-reason field. Scaling was cross-checked
against simultaneous VictronConnect readings.

On the observed device, shutdown bit 7 (`0x80`) corresponds to VictronConnect
notification `#8 Engine shutdown`. The decoder additionally returns symbolic
reason `engine_shutdown` while preserving the full raw value. Other bits remain
unnamed until supported by reliable documentation or test evidence.
