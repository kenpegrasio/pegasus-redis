#ifndef TYPES_HPP
#define TYPES_HPP

#include <string>
#include <optional>
#include <chrono>

struct Varval {
  std::string val;
  std::optional<std::chrono::time_point<std::chrono::high_resolution_clock>>
      expiry_time;

  Varval() {}
  Varval(std::string new_val, int expiry_milliseconds = -1)
      : val(std::move(new_val)) {
    if (expiry_milliseconds == -1) {
      expiry_time = std::nullopt;
    } else {
      expiry_time = std::chrono::high_resolution_clock::now() +
                    std::chrono::milliseconds(expiry_milliseconds);
    }
  }
};

#endif