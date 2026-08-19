#!/usr/bin/env bash
set -u

readonly output_dir="${BLE_ENDURANCE_OUTPUT_DIR:-/home/pi/signalk-ble-endurance}"
readonly output_file="${output_dir}/samples.jsonl"
readonly stbd_url="${BLE_GATEWAY_STBD_URL:?set BLE_GATEWAY_STBD_URL}"
readonly port_url="${BLE_GATEWAY_PORT_URL:?set BLE_GATEWAY_PORT_URL}"
readonly signalk_url="${SIGNALK_URL:-http://127.0.0.1:3000}"

mkdir -p "${output_dir}"

timestamp="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

service_json() {
  local unit="$1"
  systemctl show "${unit}" \
    --property=ActiveState \
    --property=SubState \
    --property=NRestarts \
    --property=ExecMainStartTimestamp \
    --property=ActiveEnterTimestamp \
    --no-pager 2>/dev/null |
    jq -Rn '
      reduce inputs as $line ({};
        ($line | capture("^(?<key>[^=]+)=(?<value>.*)$")) as $item |
        .[$item.key] = ($item.value | tonumber? // .)
      )'
}

read_gateway() {
  local alias="$1"
  local url="$2"
  local response_file timing
  response_file="$(mktemp)"
  if ! timing="$(curl --max-time 5 --fail --silent --show-error \
      --output "${response_file}" --write-out '%{time_total}' \
      "${url}/api/info" 2>/dev/null)"; then
    rm -f "${response_file}"
    jq -cn --arg alias "${alias}" '{gateway:$alias,reachable:false}'
    return
  fi

  jq -c --arg alias "${alias}" --arg timing "${timing}" '
    def scalar:
      if type == "string" then (tonumber? // .) else . end;
    def counters:
      split(", ") as $parts |
      reduce $parts[] as $part ({};
        ($part | capture("^(?<key>[^=]+)=(?<value>.*)$")?) as $item |
        if $item == null then .
        else .[$item.key] = ($item.value | scalar)
        end
      );
    (map({key:.name,value:.value}) | from_entries) as $s |
    ($s["BLE transport"] // "unknown") as $transport |
    {
      gateway:$alias,
      reachable:true,
      request_duration_ms:(($timing | tonumber) * 1000 | round),
      hostname:$s.Hostname,
      uptime_s:$s["Uptime (s)"],
      reset_reason:$s["Last reset reason"],
      free_heap_bytes:$s["Free memory (bytes)"],
      min_free_heap_bytes:$s["Min free memory (bytes)"],
      min_free_heap_before_reset_bytes:$s["Min free memory before last reset (bytes)"],
      wifi_rssi_db:$s["WiFi signal strength (dB)"],
      sk_connection:$s["SK connection status"],
      sk_delta_tx:$s["SK Delta TX count"],
      gateway_phase:$s["Gateway phase"],
      firmware:$s["Gateway firmware"],
      firmware_build:$s["Gateway build"],
      build_date:$s["Build date"],
      ble:{
        state:($transport | split(", ")[0]),
        counters:($transport | counters),
        raw:$transport
      }
    }' "${response_file}"
  rm -f "${response_file}"
}

stbd="$(read_gateway stbd "${stbd_url}")"
port="$(read_gateway port "${port_url}")"
signalk_service="$(service_json signalk.service)"
nodered_service="$(service_json nodered.service)"

providers_file="$(mktemp)"
devices_file="$(mktemp)"
providers_timing="$(curl --max-time 5 --fail --silent \
  --output "${providers_file}" --write-out '%{time_total}' \
  "${signalk_url}/signalk/v2/api/vessels/self/ble/_providers" 2>/dev/null || printf '')"
devices_timing="$(curl --max-time 5 --fail --silent \
  --output "${devices_file}" --write-out '%{time_total}' \
  "${signalk_url}/signalk/v2/api/vessels/self/ble/devices" 2>/dev/null || printf '')"
providers="$(jq -c . "${providers_file}" 2>/dev/null || printf '{}')"
devices="$(jq -c . "${devices_file}" 2>/dev/null || printf '[]')"
rm -f "${providers_file}" "${devices_file}"

jq -cn \
  --arg timestamp "${timestamp}" \
  --arg boot_id "$(cat /proc/sys/kernel/random/boot_id)" \
  --argjson stbd "${stbd}" \
  --argjson port "${port}" \
  --argjson signalk_service "${signalk_service}" \
  --argjson nodered_service "${nodered_service}" \
  --argjson providers "${providers}" \
  --argjson devices "${devices}" \
  --arg providers_timing "${providers_timing}" \
  --arg devices_timing "${devices_timing}" '
  {
    schema_version:2,
    timestamp:$timestamp,
    host_boot_id:$boot_id,
    services:{signalk:$signalk_service,nodered:$nodered_service},
    gateways:[$stbd,$port],
    signalk_ble:{
      provider_request_ms:(if $providers_timing == "" then null else ($providers_timing|tonumber)*1000|round end),
      device_request_ms:(if $devices_timing == "" then null else ($devices_timing|tonumber)*1000|round end),
      provider_count:($providers|length),
      device_count:($devices|length),
      observations:($devices | map(.seenBy[]? | {
        gateway:(if .providerId|endswith("Stbd") then "stbd" elif .providerId|endswith("Port") then "port" else "other" end),
        rssi_db:.rssi,
        age_s:(((now * 1000) - .lastSeen) / 1000 | floor)
      }))
    }
  }' >>"${output_file}"
