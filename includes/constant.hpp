#ifndef CONSTANT_HPP
#define CONSTANT_HPP

#include <string>

const int BUFFER_SIZE = 4096;
const std::string OK_string = "+OK\r\n";
const std::string null_bulk_string = "$-1\r\n";
const std::string empty_array = "*0\r\n";
const std::string null_array = "*-1\r\n";
const std::string zero_integer = ":0\r\n";

#endif