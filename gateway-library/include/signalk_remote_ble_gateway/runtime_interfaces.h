#pragma once

#include <cstdint>
#include <string>

namespace gateway {

struct HttpPostRequest {
  std::string url;
  std::string bearer_token;
  std::string body;
  uint32_t timeout_ms = 0;
};

struct HttpPostResponse {
  int transport_error = 0;
  int status = 0;
  uint32_t duration_ms = 0;
};

class GatewayClock {
 public:
  virtual ~GatewayClock() = default;
  virtual uint32_t now_ms() const = 0;
  virtual void delay_ms(uint32_t duration_ms) = 0;
};

class HttpTransport {
 public:
  virtual ~HttpTransport() = default;
  virtual HttpPostResponse post(const HttpPostRequest& request) = 0;
};

inline HttpPostResponse timed_http_post(HttpTransport& transport,
                                        const HttpPostRequest& request,
                                        const GatewayClock& clock) {
  const uint32_t started_ms = clock.now_ms();
  HttpPostResponse response = transport.post(request);
  response.duration_ms = clock.now_ms() - started_ms;
  return response;
}

}  // namespace gateway
