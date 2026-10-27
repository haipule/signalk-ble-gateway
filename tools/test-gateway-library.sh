#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
for test_source in "${repo_root}"/gateway-library/test/native/*_test.cpp; do
  test_name="$(basename "${test_source}" .cpp)"
  test_binary="${TMPDIR:-/tmp}/signalk-remote-ble-gateway-${test_name}"
  g++ -std=c++17 -Wall -Wextra -Werror -pedantic \
    -I"${repo_root}/gateway-library/include" \
    "${test_source}" \
    -o "${test_binary}"
  "${test_binary}"
done
