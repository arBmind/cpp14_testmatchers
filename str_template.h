#pragma once
#include <string>
#include <tuple>
#include <utility>

#include "str_represent.h"
#include "str_concat.h"

void str_replace_all(std::string& str, const std::string& key, const std::string& subst) {
  size_t index = 0;
  while (true) {
    index = str.find(key, index);
    if (index == std::string::npos) break;

    str.replace(index, key.length(), subst);
    index += subst.length();
  }
}

namespace details {

std::string make_key(size_t i) {
  return str_concat("#{", i, "}");
}

template <class... T, size_t... I>
std::string str_template(std::string str, const std::tuple<T...>& args, std::index_sequence<I...>) {
  auto x = {(str_replace_all(str, make_key(I), str_represent(std::get<I>(args))), 0)...};
  (void)x;
  return str;
}

} // namespace details

template <class... T>
std::string str_template(const std::string& src, const std::tuple<T...>& args) {
  return details::str_template(src, args, std::make_index_sequence<sizeof... (T)>());
}
