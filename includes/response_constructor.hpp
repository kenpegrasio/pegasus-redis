#ifndef RESPONSE_CONSTRUCTOR_HPP
#define RESPONSE_CONSTRUCTOR_HPP

#include <string>
#include <algorithm>
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

#endif