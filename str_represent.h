#pragma once
#include <string>
#include <array>
#include <sstream>
#include <cstddef>
#include "str_concat.h"

template <class T>
std::string str_represent(const T& t) {
  return std::to_string(t);
}

std::string str_represent(std::nullptr_t) {
  return "<nullptr>";
}

std::string str_represent(bool b) {
  return b ? "<true>" : "<false>";
}

std::string str_represent(const char* str) {
  return str_concat('"', str, '"');
}

std::string str_represent(const std::string& str) {
  return str_concat('"', str, '"');
}

template <class T, size_t N>
std::string str_represent(const std::array<T, N>& arr) {
  std::stringstream ss;
  ss << '[';
  if (N != 0) {
    for (const T& t : arr) {
      ss << str_represent(t) << ',' << ' ';
    }
    ss.seekp(-2, std::ios_base::cur);
  }
  ss << ']';
  return ss.str();
}
