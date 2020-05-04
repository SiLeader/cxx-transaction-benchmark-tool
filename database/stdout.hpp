//
// Created by cerussite on 5/4/20.
//

#pragma once

#include <iostream>
#include <string_view>

#include "../executor.hpp"

namespace tb::database {

class StdoutTestDatabase {
 public:
  void execute(std::string_view query) { std::cout << query << std::endl; }

  static StdoutTestDatabase Make() { return StdoutTestDatabase(); }

 public:
  static Executor<StdoutTestDatabase> CreateExecutor() {
    return Executor<StdoutTestDatabase>();
  }
};

}  // namespace tb::database
