#pragma once
#include <string>
#include <sstream>

template <class... T>
std::string str_concat(const T&... t) {
  std::stringstream ss;
  auto x = {(ss << t, 0) ...};
  (void)x;
  return ss.str();
}
