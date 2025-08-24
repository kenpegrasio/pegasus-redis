#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <thread>
#include <vector>

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

const int BUFFER_SIZE = 4096;

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

const std::string OK_string = "+OK\r\n";
const std::string null_bulk_string = "$-1\r\n";

std::string construct_bulk_string(std::string str) {
  std::string response = "$";
  int sz = str.size();
  std::vector<int> digits;
  while (sz > 0) {
    digits.push_back(sz % 10);
    sz = sz / 10;
  }
  reverse(digits.begin(), digits.end());
  for (auto digit : digits) {
    response.push_back(digit + '0');
  }
  response += "\r\n";
  response += str;
  response += "\r\n";
  return response;
}

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

void process_client(int client_socket) {
  std::map<std::string, Varval> variables;
  while (true) {
    std::string res = read_request(client_socket);
    if (res == "") return;

    int ptr = 0;
    auto elements = read_array(res, ptr);

    // std::cout << "Buffer: " << std::endl;
    // for (auto c : std::string(buffer)) {
    //   std::cout << std::hex << static_cast<int>(c) << " ";
    // }
    // std::cout << std::endl;

    if (elements[0] == "PING") {
      std::string response = "+PONG\r\n";
      send(client_socket, response.c_str(), response.size(), 0);
    } else if (elements[0] == "ECHO") {
      std::string response = construct_bulk_string(elements[1]);
      send(client_socket, response.c_str(), response.size(), 0);
    } else if (elements[0] == "SET") {
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
    } else if (elements[0] == "GET") {
      if (variables.find(elements[1]) == variables.end()) {
        send(client_socket, null_bulk_string.c_str(), null_bulk_string.size(),
             0);
      } else {
        auto val = variables[elements[1]];
        if (val.expiry_time && *val.expiry_time <= std::chrono::high_resolution_clock::now()) {
          variables.erase(elements[1]);
          send(client_socket, null_bulk_string.c_str(), null_bulk_string.size(),
               0);
        } else {
          std::string response = construct_bulk_string(val.val);
          send(client_socket, response.c_str(), response.size(), 0);
        }
      }
    } else {
      std::string response = "+PONG\r\n";
      send(client_socket, response.c_str(), response.size(), 0);
    }
  }
}

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    std::cerr << "Failed to bind to port 6379\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  std::cout << "Waiting for a client to connect...\n";

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  while (true) {
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);
    std::cout << "Client connected\n";
    std::thread t(process_client, client_fd);
    t.detach();
  }
  close(server_fd);
  return 0;
}
