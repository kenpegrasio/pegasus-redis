#ifndef TYPES_HPP
#define TYPES_HPP

#include <chrono>
#include <optional>
#include <string>

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

template <typename T>
class CircularBuffer {
 private:
  std::vector<T> buffer;
  int cur_size;
  int front_idx;

  void resize() {
    int capacity = buffer.size();
    std::vector<T> new_buffer(capacity * 2);
    for (int idx = 0; idx < cur_size; idx++) {
      new_buffer[front_idx + idx] = buffer[(front_idx + idx) % capacity];
    }
    buffer = std::move(new_buffer);
    front_idx = 0;
  }

 public:
  CircularBuffer(int initial_size = 4)
      : buffer(initial_size), cur_size(0), front_idx(0) {}

  int size() { return cur_size; }

  void push_back(T new_element) {
    if (cur_size == buffer.size()) resize();
    buffer[(front_idx + cur_size) % (int)buffer.size()] = new_element;
    cur_size++;
  }

  void push_front(T new_element) {
    if (cur_size == buffer.size()) resize();
    front_idx = (front_idx - 1 + (int)buffer.size()) % (int)buffer.size();
    buffer[front_idx] = new_element;
    cur_size++;
  }

  T back() {
    if (cur_size == 0) throw std::string("Buffer is empty");
    return buffer[(front_idx + cur_size - 1) % (int)buffer.size()];
  }

  void pop_back() {
    if (cur_size == 0) return;
    cur_size--;
  }

  T front() {
    if (cur_size == 0) throw std::string("Buffer is empty");
    return buffer[front_idx];
  }

  void pop_front() {
    if (cur_size == 0) return;
    front_idx = (front_idx + 1) % (int)buffer.size();
    cur_size--;
  }

  T& operator[](int idx) {
    if (idx < 0 || idx >= cur_size) {
      throw std::string("Index out of range");
    }
    return buffer[(front_idx + idx) % (int)buffer.size()];
  }
};

#endif