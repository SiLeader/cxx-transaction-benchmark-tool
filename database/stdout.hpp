//
// Created by cerussite on 5/4/20.
//

#pragma once

#include <iostream>
#include <properties.hpp>
#include <string_view>

#include "database.hpp"

namespace tb::database {

class StdoutTestDatabase : public Database {
 public:
  void execute(std::string_view query) override {
    std::cout << query << std::endl;
  }

 public:
  static std::unique_ptr<Database> Make(const Properties&) {
    return std::make_unique<StdoutTestDatabase>();
  }
};

}  // namespace tb::database
