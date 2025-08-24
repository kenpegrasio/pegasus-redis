#ifndef REQUEST_HANDLER_HPP
#define REQUEST_HANDLER_HPP

#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "types.hpp"
#include "constant.hpp"

bool is_number(char x) { return x >= '0' && x <= '9'; }

unsigned long long read_size(std::string &buffer, int &ptr) {
  unsigned long long ans = 0;
  while (is_number(buffer[ptr])) {
    ans = ans * 10 + int(buffer[ptr] - '0');
    ptr++;
  }
  return ans;
}

std::string read_string(std::string &buffer, int &ptr) {
  ptr++;
  auto str_len = read_size(buffer, ptr);
  ptr += 2;
  std::string res = "";
  for (int i = 0; i < str_len; i++) {
    res += buffer[ptr];
    ptr++;
  }
  return res;
}

std::vector<std::string> read_array(std::string &buffer, int &ptr) {
  ptr++;
  int number_of_element = read_size(buffer, ptr);
  ptr += 2;
  std::vector<std::string> res;
  for (int i = 0; i < number_of_element; i++) {
    auto str = read_string(buffer, ptr);
    ptr += 2;
    res.push_back(str);
  }
  return res;
}

std::string read_request(int client_socket) {
  std::string res = "";
  char buffer[BUFFER_SIZE] = {0};
  int bytes_read = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
  if (bytes_read == 0) {
    std::cerr << "Empty request" << std::endl;
    close(client_socket);
    return "";
  }
  if (bytes_read <= 0) {
    std::cerr << "Failed to receive data from client" << std::endl;
    close(client_socket);
    return "";
  }
  buffer[bytes_read] = '\0';
  res += buffer;
  return res;
}

#endif