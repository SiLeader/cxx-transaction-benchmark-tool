//
// Created by cerussite on 4/26/20.
//

#pragma once

#include <functional>
#include <unordered_map>
#include <variant>

namespace tb {

namespace te {

using Id = std::string;

inline constexpr std::size_t kNumberIndex = 0, kStringIndex = 1;
using Value = std::variant<std::int_fast64_t, std::string>;

using Valuable = Value;

struct Function {
  Id name;
  std::vector<Valuable> args;
};

using Expression = std::variant<Function, Value>;

using RawString = char;
using Statement = std::vector<std::variant<RawString, Expression>>;

class FunctionContainer {
 public:
  using FunctionType = std::function<Value(const std::vector<Value>&)>;

 private:
  std::unordered_map<std::string, FunctionType> functions_;

 public:
  template <class F>
  void add(std::string name, F&& f) {
    functions_[std::move(name)] = std::forward<F>(f);
  }

  void addAll(const std::vector<std::pair<std::string, FunctionType>>& fs) {
    for (const auto& f : fs) {
      add(f.first, f.second);
    }
  }

 public:
  bool has(const std::string& name) const {
    return functions_.find(name) != std::end(functions_);
  }

  const FunctionType& operator[](const std::string& name) const {
    return functions_.at(name);
  }
};

}  // namespace te

std::string CreateString(
    const std::string& template_string,
    const std::vector<std::pair<
        std::string, te::FunctionContainer::FunctionType>>& functions = {});

}  // namespace tb
