#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace gateway {

enum class HttpBatchResult {
  kDelivered,
  kDropInvalid,
  kRetry,
};

inline HttpBatchResult classify_http_status(const int status) {
  if (status >= 200 && status < 300) {
    return HttpBatchResult::kDelivered;
  }
  if (status == 400) {
    return HttpBatchResult::kDropInvalid;
  }
  return HttpBatchResult::kRetry;
}

inline uint32_t next_retry_interval(const uint32_t current,
                                    const uint32_t normal,
                                    const uint32_t maximum,
                                    const bool request_completed) {
  if (request_completed) {
    return normal;
  }
  if (current >= maximum || current > std::numeric_limits<uint32_t>::max() / 2) {
    return maximum;
  }
  return std::min(current * 2, maximum);
}

inline bool normalize_mac_address(const std::string_view input,
                                  std::string* output) {
  if (output == nullptr) {
    return false;
  }
  const auto first = input.find_first_not_of(" \t\r\n");
  if (first == std::string_view::npos) {
    output->clear();
    return false;
  }
  const auto last = input.find_last_not_of(" \t\r\n");
  const std::string_view trimmed = input.substr(first, last - first + 1);
  if (trimmed.size() != 17) {
    output->clear();
    return false;
  }

  output->resize(trimmed.size());
  for (size_t index = 0; index < trimmed.size(); ++index) {
    const char value = trimmed[index];
    if ((index + 1) % 3 == 0) {
      if (value != ':' && value != '-') {
        output->clear();
        return false;
      }
      (*output)[index] = ':';
    } else {
      if (!std::isxdigit(static_cast<unsigned char>(value))) {
        output->clear();
        return false;
      }
      (*output)[index] = static_cast<char>(
          std::toupper(static_cast<unsigned char>(value)));
    }
  }
  return true;
}

inline bool mac_is_allowed(const std::string_view address,
                           const std::vector<std::string>& allowed_macs) {
  if (allowed_macs.empty()) {
    return true;
  }
  std::string normalized;
  if (!normalize_mac_address(address, &normalized)) {
    return false;
  }
  return std::find(allowed_macs.begin(), allowed_macs.end(), normalized) !=
         allowed_macs.end();
}

template <typename Advertisement>
class BoundedBatchQueue {
 public:
  enum class PushResult {
    kAccepted,
    kAcceptedAfterDroppingOldest,
  };

  BoundedBatchQueue(const size_t maximum_pending, const size_t maximum_batch)
      : maximum_pending_(maximum_pending), maximum_batch_(maximum_batch) {
    pending_.reserve(maximum_pending_);
    inflight_.reserve(maximum_batch_);
  }

  bool valid() const { return maximum_pending_ > 0 && maximum_batch_ > 0; }

  PushResult push(Advertisement advertisement) {
    PushResult result = PushResult::kAccepted;
    if (pending_.size() >= maximum_pending_) {
      pending_.erase(pending_.begin());
      result = PushResult::kAcceptedAfterDroppingOldest;
    }
    pending_.push_back(std::move(advertisement));
    return result;
  }

  bool prepare() {
    if (!inflight_.empty()) {
      return true;
    }
    if (pending_.empty()) {
      return false;
    }
    const size_t count = std::min(pending_.size(), maximum_batch_);
    inflight_.assign(pending_.begin(), pending_.begin() + count);
    pending_.erase(pending_.begin(), pending_.begin() + count);
    inflight_sequence_ = next_sequence_++;
    return true;
  }

  size_t acknowledge() {
    const size_t count = inflight_.size();
    inflight_.clear();
    return count;
  }

  size_t drop_invalid() { return acknowledge(); }

  size_t size() const { return pending_.size() + inflight_.size(); }
  size_t pending_size() const { return pending_.size(); }
  size_t inflight_size() const { return inflight_.size(); }
  uint32_t inflight_sequence() const { return inflight_sequence_; }
  const std::vector<Advertisement>& inflight() const { return inflight_; }

 private:
  size_t maximum_pending_;
  size_t maximum_batch_;
  uint32_t next_sequence_ = 0;
  uint32_t inflight_sequence_ = 0;
  std::vector<Advertisement> pending_;
  std::vector<Advertisement> inflight_;
};

}  // namespace gateway
