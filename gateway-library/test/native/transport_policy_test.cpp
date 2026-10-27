#include "signalk_remote_ble_gateway/transport_policy.h"

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace {

void test_mac_normalization() {
  std::string output;
  assert(gateway::normalize_mac_address(" aa-bb-cc-dd-ee-ff ", &output));
  assert(output == "AA:BB:CC:DD:EE:FF");
  assert(gateway::normalize_mac_address("01:23:45:67:89:ab", &output));
  assert(output == "01:23:45:67:89:AB");
  assert(!gateway::normalize_mac_address("01:23:45:67:89", &output));
  assert(!gateway::normalize_mac_address("01:23:45:67:89:ZZ", &output));

  const std::vector<std::string> allowlist = {"AA:BB:CC:DD:EE:FF"};
  assert(gateway::mac_is_allowed("aa-bb-cc-dd-ee-ff", allowlist));
  assert(!gateway::mac_is_allowed("01:23:45:67:89:AB", allowlist));
  assert(!gateway::mac_is_allowed("not-a-mac", allowlist));
  assert(gateway::mac_is_allowed("not-a-mac", {}));
}

void test_queue_bounds_and_batches() {
  gateway::BoundedBatchQueue<int> queue(3, 2);
  assert(queue.valid());
  assert(queue.push(1) == decltype(queue)::PushResult::kAccepted);
  queue.push(2);
  queue.push(3);
  assert(queue.push(4) ==
         decltype(queue)::PushResult::kAcceptedAfterDroppingOldest);
  assert(queue.size() == 3);

  assert(queue.prepare());
  assert(queue.inflight_sequence() == 0);
  assert(queue.inflight_size() == 2);
  assert(queue.inflight()[0] == 2);
  assert(queue.inflight()[1] == 3);
  assert(queue.pending_size() == 1);

  // A retry retains the exact in-flight batch and sequence.
  assert(queue.prepare());
  assert(queue.inflight_sequence() == 0);
  assert(queue.inflight()[0] == 2);
  assert(queue.acknowledge() == 2);

  assert(queue.prepare());
  assert(queue.inflight_sequence() == 1);
  assert(queue.inflight()[0] == 4);
  assert(queue.drop_invalid() == 1);
  assert(queue.size() == 0);
  assert(!queue.prepare());
}

void test_retry_and_http_policy() {
  assert(gateway::classify_http_status(204) ==
         gateway::HttpBatchResult::kDelivered);
  assert(gateway::classify_http_status(400) ==
         gateway::HttpBatchResult::kDropInvalid);
  assert(gateway::classify_http_status(401) ==
         gateway::HttpBatchResult::kRetry);
  assert(gateway::classify_http_status(503) ==
         gateway::HttpBatchResult::kRetry);

  assert(gateway::next_retry_interval(2000, 2000, 30000, false) == 4000);
  assert(gateway::next_retry_interval(16000, 2000, 30000, false) == 30000);
  assert(gateway::next_retry_interval(30000, 2000, 30000, false) == 30000);
  assert(gateway::next_retry_interval(UINT32_MAX, 2000, 30000, false) ==
         30000);
  assert(gateway::next_retry_interval(16000, 2000, 30000, true) == 2000);
}

}  // namespace

int main() {
  test_mac_normalization();
  test_queue_bounds_and_batches();
  test_retry_and_http_policy();
}
