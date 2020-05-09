//
// Created by cerussite on 5/9/20.
//

#pragma once

#include <memory>
#include <string_view>

namespace tb::database {

class Database {
 public:
  virtual ~Database() = default;

 public:
  virtual void execute(std::string_view) = 0;
};

}  // namespace tb::database
