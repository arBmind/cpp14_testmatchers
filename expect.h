#pragma once
#include <utility>
#include <tuple>
#include <iostream>
#include <sstream>
#include <memory>
#include <array>
#include <cstring>

#include "str_represent.h"
#include "str_concat.h"

namespace expect {

template <class L, class R>
bool is_equal(const L& l, const R& r) {
  return l == r;
}

bool is_equal(const char* l, const char* r) {
  return 0 == strcmp(l, r);
}

void failed_report(const std::string& message, const std::string& actual,
                   const char* combine, const std::string& matcher) {
  std::cout << "\n"
               "expectation failed!\n"
               "------------------\n";
  if (!message.empty()) std::cout << message << '\n';
  std::cout << actual << '\n'
            << combine << matcher << '\n';
}

struct MatcherResult {
  bool success;
  std::string description;
};

template <class Actual>
struct Expector {
  template<class Matcher>
  void to(Matcher&& matcher, const std::string& message = {}) const {
    auto result = matcher(actual);
    if (result.success) return;
    failed_report(message, str_concat("expected: ", str_represent(actual)), "to ", result.description);
  }
  template<class Matcher>
  void not_to(Matcher&& matcher, const std::string& message = {}) const {
    auto result = matcher(actual);
    if (!result.success) return;
    failed_report(message, str_concat("expected: ", str_represent(actual)), "not to ", result.description);
  }
  template<class Matcher>
  auto to_not(Matcher&& matcher, const std::string& message = {}) const {
    return not_to((Matcher&&)matcher, message);
  }

  Actual actual;
};

template <class Actual>
auto expect(Actual&& actual) {
  return Expector<Actual>{ (Actual&&)actual };
}

//
// matchers
//
template <class T>
auto equal(T&& expected) {
  return [cap_expected = (T&&)expected](const auto& actual) {
    return MatcherResult { is_equal(cap_expected, actual),
          str_concat("equal: ", str_represent(cap_expected)) };
  };
}

template <class Mini, class Maxi>
struct BeBetween {
  auto inclusive() const {
    return [cap_min = (Mini&&)mini, cap_max = (Maxi&&)maxi](const auto& actual) {
      return MatcherResult { cap_min <= actual && cap_max >= actual,
            str_concat("be between ", str_represent(cap_min), " and ", str_represent(cap_max), " (inclusive)")
      };
    };
  }

  auto exclusive() const {
    return [cap_min = (Mini&&)mini, cap_max = (Maxi&&)maxi](const auto& actual) {
      return MatcherResult { cap_min < actual && cap_max > actual,
            str_concat("be between ", str_represent(cap_min), " and ", str_represent(cap_max), " (exclusive)")
      };
    };
  }

  template <class Actual>
  auto operator() (const Actual& actual) const {
    return inclusive()(actual);
  }

  Mini mini;
  Maxi maxi;
};

template <class Mini, class Maxi>
auto be_between(Mini&& mini, Maxi&& maxi) {
  return BeBetween<Mini, Maxi> { (Mini&&)mini, (Maxi&&)maxi };
}


auto be_truthy = [](const auto& actual) {
  return MatcherResult { (bool)actual, "be truthy" };
};
auto be_falsey = [](const auto& actual) {
  return MatcherResult { !(bool)actual, "be falsey" };
};
auto be_nullptr = [](const auto& actual) {
  return MatcherResult { nullptr == actual, "be nullptr" };
};

template <class Matcher>
auto all(Matcher&& matcher) {
  return [cap_matcher = (Matcher&&)matcher](const auto& actual) {
    auto b = std::begin(actual);
    auto e = std::end(actual);
    if (b == e) {
      return MatcherResult { true, str_represent("all be <unknown>") };
    }
    while (true) {
      auto res = cap_matcher(*b);
      if (!res.success) return MatcherResult { false, str_concat("all ", res.description) };
      b++;
      if (b == e) {
        return MatcherResult { true, str_concat("all ", res.description) };
      }
    }
  };
}

void runTest() {
  expect(2+2).to(equal(5), "calculation");
  expect("hello").to(equal("hello"));
  expect("hello").to(equal("some"));
  expect(0).to(be_truthy);
  expect(false).not_to(be_falsey);
  expect(nullptr).not_to(be_nullptr);
  expect(2+2).to(be_between(4, 5).inclusive());
  expect(2+2).to(be_between(4, 5).exclusive());
  auto arr = std::array<int, 2>{{3, 4}};
  expect(arr).not_to(all(be_between(3, 4)));
}

} // namespace expect
