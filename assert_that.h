#pragma once
#include <functional>
#include <algorithm>
#include <iostream>
#include <utility>

#include "str_represent.h"
#include "str_concat.h"
#include "str_template.h"

namespace assert_that {

auto delayed_logical_and = [](auto&& l, auto&& r) { return l() && r(); };
auto delayed_logical_or = [](auto&& l, auto&& r) { return l() || r(); };

template<class LeftMatcher>
struct CombineMatchers {

  template<class Op, class RightMatcher>
  struct Binary : CombineMatchers<Binary<Op, RightMatcher>> {
    Binary(const char* op_name, Op&& op, LeftMatcher&& left, RightMatcher&& right)
      : op_name(op_name), op((Op&&)op), left((LeftMatcher&&)left), right((RightMatcher&&)right) {}

    template<class Actual>
    auto matches(const Actual& actual) const {
      return op([&]{ return left.matches(actual); }, [&]{ return right.matches(actual); });
    }

    auto description() const {
      return str_concat(left.description(), op_name, right.description());
    }

    const char* op_name;
    Op op;
    LeftMatcher left;
    RightMatcher right;
  };

  template<class RightMatcher>
  auto And(RightMatcher&& right) {
    auto op = delayed_logical_and;
    return Binary<decltype(op), RightMatcher>(" and is ",
                                              (decltype(op)&&)op, (LeftMatcher&&)*this, (RightMatcher&&)right);
  }
  template<class RightMatcher>
  auto operator&& (RightMatcher&& right) {
    return And((RightMatcher&&)right);
  }

  template<class RightMatcher>
  auto Or (RightMatcher&& right) {
    auto op = delayed_logical_or;
    return Binary<decltype(op), RightMatcher>(" or is ",
                                              (decltype(op)&&)op, (LeftMatcher&&)*this, (RightMatcher&&)right);
  }
  template<class RightMatcher>
  auto operator|| (RightMatcher&& right) {
    return Or((RightMatcher&&)right);
  }
};

template<class DescriptionF, class Op, class... Args>
struct OpMatcher : CombineMatchers<OpMatcher<DescriptionF, Op, Args...>> {
  OpMatcher(DescriptionF&& description_f, Op&& op, std::tuple<Args...>&& args)
    : description_f((DescriptionF&&)description_f)
    , op((Op&&)op)
    , args((std::tuple<Args...>&&)args) {}

  template<class Actual>
  auto matches(const Actual& actual) const {
    return matches_impl(actual, std::make_index_sequence<sizeof...(Args)>());
  }

  auto description() const {
    return description_f(args);
  }

private:
  template<class Actual, size_t... I>
  auto matches_impl(const Actual& actual, std::index_sequence<I...>) const {
    return op(actual, std::get<I>(args)...);
  }

  DescriptionF description_f;
  Op op;
  std::tuple<Args...> args;
};

template<class Op, class... Args>
auto make_matcher(const char* description, Op&& op, std::tuple<Args...>&& args) {
  auto builder = [=](const auto& args){
    return str_template(description, args);
  };
  using builder_t = decltype(builder);
  return OpMatcher<builder_t, Op, Args...>((builder_t&&)builder, (Op&&)op, (std::tuple<Args...>&&)args);
}

template<class Op>
auto make_matcher(const char* name, Op&& op) {
  auto builder = [=](const auto&){
    return name;
  };
  using builder_t = decltype(builder);
  return OpMatcher<builder_t, Op>((builder_t&&)builder, (Op&&)op, std::tuple<>());
}

struct CompareMatchers {

  template<class Value>
  auto equal_to(Value&& value) {
    return make_matcher("equal to #{0}", std::equal_to<>(), std::make_tuple(value));
  }
  template<class Value>
  auto operator==(Value&& value) { return equal_to((Value&&)value); }

  template<class Value>
  auto greater_than(Value&& value) {
    return make_matcher("greater than #{0}", std::greater<>(), std::make_tuple(value));
  }
  template<class Value>
  auto operator>(Value&& value) { return greater_than((Value&&)value); }

  template<class Value>
  auto less_than(Value&& value) {
    return make_matcher("less than #{0}", std::less<>(), std::make_tuple(value));
  }
  template<class Value>
  auto operator<(Value&& value) { return less_than((Value&&)value); }
};

struct SpecialValueMatchers {

  auto nil() {
    return make_matcher("null", [](const auto& value) { return value == nullptr; });
  }
  auto truthy() {
    return make_matcher("truthy", [](const auto& value) { return value == true; });
  }
  auto falsey() {
    return make_matcher("falsey", [](const auto& value) { return value == false; });;
  }
};

struct Asserter {
  template<class Actual, class Matcher>
  auto that(const Actual& actual, const Matcher& matcher) {
    if (matcher.matches(actual)) return;
    std::cout << "\n"
                 "assert failed!\n"
                 "--------------\n"
                 "asserted that " << str_represent(actual) << "\n"
                 " is " << matcher.description() << '\n';
  }
};
auto assert = Asserter{};

template<class... Part>
struct MatcherCollection : Part... {};

auto is = MatcherCollection<CompareMatchers, SpecialValueMatchers>{};

void runTest() {
  assert.that(8, is.greater_than(7) && is.less_than(9));
  assert.that(8, is > 8 && is < 9);
  assert.that(nullptr, is.nil());
}

} // namespace AssertThat
