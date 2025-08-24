#ifndef RESPONSE_CONSTRUCTOR_HPP
#define RESPONSE_CONSTRUCTOR_HPP

#include <algorithm>
#include <string>
#include <vector>

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

std::string construct_integer(int val) {
  std::string response = ":";
  if (val < 0) response += "-";
  std::vector<int> digits;
  while (val > 0) {
    digits.push_back(val % 10);
    val = val / 10;
  }
  reverse(digits.begin(), digits.end());
  for (auto digit : digits) {
    response.push_back(digit + '0');
  }
  response += "\r\n";
  return response;
}

std::string construct_array(std::vector<std::string> v) {
  std::string response = "*";
  response += std::to_string(v.size());
  response += "\r\n";
  for (auto element : v) {
    response += construct_bulk_string(element);
  }
  return response;
}

#endif