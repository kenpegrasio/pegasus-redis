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
                  std::map<std::string, std::vector<std::string>> &lists,
                  std::vector<std::string> &elements) {
  if ((int)elements.size() < 3) throw std::string("Invalid RPUSH operation");
  for (int i = 2; i < (int)elements.size(); i++) {
    lists[elements[1]].push_back(elements[i]);
  }
  std::string response = construct_integer(lists[elements[1]].size());
  send(client_socket, response.c_str(), response.size(), 0);
}

#endif