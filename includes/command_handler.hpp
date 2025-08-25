#ifndef COMMAND_HANDLER_HPP
#define COMMAND_HANDLER_HPP

#include <sys/socket.h>

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include "constant.hpp"
#include "response_constructor.hpp"
#include "types.hpp"

void handle_ping(int client_socket) {
  std::string response = "+PONG\r\n";
  send(client_socket, response.c_str(), response.size(), 0);
}

void handle_echo(int client_socket, std::vector<std::string> &elements) {
  std::string response = construct_bulk_string(elements[1]);
  send(client_socket, response.c_str(), response.size(), 0);
}

void handle_set(int client_socket, std::map<std::string, Varval> &variables,
                std::vector<std::string> &elements) {
  if ((int)elements.size() < 3) throw std::string("Invalid SET operation");
  if (elements.size() == 5) {
    if (elements[3] == "px") {
      variables[elements[1]] = Varval(elements[2], std::stoi(elements[4]));
    } else {
      throw std::string("No px identifier found");
    }
  }
  if (elements.size() == 3) {
    variables[elements[1]] = Varval(elements[2]);
  }
  send(client_socket, OK_string.c_str(), OK_string.size(), 0);
}

void handle_get(int client_socket, std::map<std::string, Varval> &variables,
                std::vector<std::string> &elements) {
  if (variables.find(elements[1]) == variables.end()) {
    send(client_socket, null_bulk_string.c_str(), null_bulk_string.size(), 0);
  } else {
    auto val = variables[elements[1]];
    if (val.expiry_time &&
        *val.expiry_time <= std::chrono::high_resolution_clock::now()) {
      variables.erase(elements[1]);
      send(client_socket, null_bulk_string.c_str(), null_bulk_string.size(), 0);
    } else {
      std::string response = construct_bulk_string(val.val);
      send(client_socket, response.c_str(), response.size(), 0);
    }
  }
}

void handle_rpush(int client_socket,
                  std::map<std::string, CircularBuffer<std::string>> &lists,
                  std::vector<std::string> &elements) {
  if ((int)elements.size() < 3) throw std::string("Invalid RPUSH operation");
  for (int i = 2; i < (int)elements.size(); i++) {
    lists[elements[1]].push_back(elements[i]);
  }
  std::string response = construct_integer(lists[elements[1]].size());
  send(client_socket, response.c_str(), response.size(), 0);
}

void handle_lpush(int client_socket,
                  std::map<std::string, CircularBuffer<std::string>> &lists,
                  std::vector<std::string> &elements) {
  if ((int)elements.size() < 3) throw std::string("Invalid LPUSH operation");
  for (int i = 2; i < (int)elements.size(); i++) {
    lists[elements[1]].push_front(elements[i]);
  }
  std::string response = construct_integer(lists[elements[1]].size());
  send(client_socket, response.c_str(), response.size(), 0);
}

void handle_lrange(int client_socket,
                   std::map<std::string, CircularBuffer<std::string>> &lists,
                   std::vector<std::string> &elements) {
  if ((int)elements.size() < 4) throw std::string("Invalid LRANGE operation");
  if (lists.find(elements[1]) == lists.end()) {
    send(client_socket, empty_array.c_str(), empty_array.size(), 0);
    return;
  }
  int l = std::stoi(elements[2]);
  int r = std::stoi(elements[3]);
  r = std::min(r, (int)lists[elements[1]].size() - 1);
  if (l < 0) l = std::max(0, (int)lists[elements[1]].size() + l);
  if (r < 0) r = std::max(0, (int)lists[elements[1]].size() + r);
  std::vector<std::string> res;
  for (int i = l; i <= r; i++) {
    res.push_back(lists[elements[1]][i]);
  }
  std::string response = construct_array(res);
  send(client_socket, response.c_str(), response.size(), 0);
}

void handle_llen(int client_socket,
                 std::map<std::string, CircularBuffer<std::string>> &lists,
                 std::vector<std::string> &elements) {
  if (elements.size() != 2) throw std::string("Invalid LLEN operation");
  if (lists.find(elements[1]) == lists.end()) {
    send(client_socket, zero_integer.c_str(), zero_integer.size(), 0);
  } else {
    std::string response = construct_integer(lists[elements[1]].size());
    send(client_socket, response.c_str(), response.size(), 0);
  }
}

void handle_lpop(int client_socket,
                 std::map<std::string, CircularBuffer<std::string>> &lists,
                 std::vector<std::string> &elements) {
  if (elements.size() != 2) throw std::string("Invalid LPOP operation");
  if (lists.find(elements[1]) == lists.end() || lists[elements[1]].size() == 0) {
    send(client_socket, null_bulk_string.c_str(), null_bulk_string.size(), 0);
  } else {
    std::string response = construct_bulk_string(lists[elements[1]].front());
    lists[elements[1]].pop_front();
    send(client_socket, response.c_str(), response.size(), 0);
  }
}

#endif