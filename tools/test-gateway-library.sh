#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
test_binary="${TMPDIR:-/tmp}/signalk-remote-ble-gateway-policy-test"

g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
  -I"${repo_root}/gateway-library/include" \
  "${repo_root}/gateway-library/test/native/transport_policy_test.cpp" \
  -o "${test_binary}"
"${test_binary}"
