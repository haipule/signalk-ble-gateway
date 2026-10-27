#include "signalk_remote_ble_gateway/runtime_interfaces.h"

#include <cassert>
#include <cstdint>

namespace {

class FakeClock final : public gateway::GatewayClock {
 public:
  uint32_t now_ms() const override { return now; }

  void delay_ms(const uint32_t duration_ms) override {
    last_delay_ms = duration_ms;
    now += duration_ms;
  }

  mutable uint32_t now = 0;
  uint32_t last_delay_ms = 0;
};

class FakeHttpTransport final : public gateway::HttpTransport {
 public:
  explicit FakeHttpTransport(FakeClock& clock) : clock_(clock) {}

  gateway::HttpPostResponse post(
      const gateway::HttpPostRequest& request) override {
    last_request = request;
    clock_.now += elapsed_ms;
    return {transport_error, status, 0};
  }

  FakeClock& clock_;
  gateway::HttpPostRequest last_request;
  uint32_t elapsed_ms = 0;
  int transport_error = 0;
  int status = 0;
};

void test_request_execution_and_timing() {
  FakeClock clock;
  clock.now = 100;
  FakeHttpTransport transport(clock);
  transport.elapsed_ms = 27;
  transport.transport_error = -1;
  transport.status = 503;

  gateway::HttpPostRequest request;
  request.url = "http://signalk.test/advertisements";
  request.bearer_token = "provider-token";
  request.body = "{\"gateway_id\":\"test\"}";
  request.timeout_ms = 5000;

  const gateway::HttpPostResponse response =
      gateway::timed_http_post(transport, request, clock);

  assert(transport.last_request.url == request.url);
  assert(transport.last_request.bearer_token == request.bearer_token);
  assert(transport.last_request.body == request.body);
  assert(transport.last_request.timeout_ms == request.timeout_ms);
  assert(response.transport_error == -1);
  assert(response.status == 503);
  assert(response.duration_ms == 27);

  clock.delay_ms(2000);
  assert(clock.last_delay_ms == 2000);
  assert(clock.now == 2127);
}

void test_timing_wraparound() {
  FakeClock clock;
  clock.now = UINT32_MAX - 4;
  FakeHttpTransport transport(clock);
  transport.elapsed_ms = 10;

  gateway::HttpPostRequest request;
  const gateway::HttpPostResponse response =
      gateway::timed_http_post(transport, request, clock);

  assert(response.duration_ms == 10);
  assert(clock.now == 5);
}

}  // namespace

int main() {
  test_request_execution_and_timing();
  test_timing_wraparound();
  return 0;
}
