#include <iostream>

#include "../database/stdout.hpp"
#include "configuration.hpp"
#include "executor.hpp"

int main() {
  auto config = tb::Configuration::Make("workloads/test.yml");

  auto executor = tb::database::StdoutTestDatabase::CreateExecutor();
  auto result = executor.execute(config);

  result.dump(std::cout);

  return 0;
}
