#include <argparse.hpp>
#include <configuration.hpp>
#include <database_creator.hpp>
#include <iostream>

#include "executor.hpp"

namespace {

template <class F>
tb::Statistics Execute(const tb::Configuration& config,
                       const tb::Properties& props, F&& f) {
  tb::Executor executor(std::forward<F>(f));
  return executor.execute(config, props);
}

tb::Statistics ExecuteByName(const std::string& name,
                             const tb::Configuration& config,
                             const tb::Properties& props) {
  return Execute(config, props, tb::database::GetDatabaseCreator(name));
}

}  // namespace

int main(const int argc, const char* const* const argv) {
  argparse::ArgumentParser parser("tx-bench");
  parser.addArgument({"--workload", "-w"}, "workload configuration file");
  parser.addArgument({"--threads"}, "thread count (overwrite configuration)");
  parser.addArgument({"--result", "-r"}, "result output (default: stdout)");
  parser.addArgument({"--properties", "-p"}, "properties file");
  parser.addArgument({"--database", "--db", "-d"}, "database name");

  const auto args = parser.parseArgs(argc, argv);

  std::string workload;
  if (!args.get("workload", workload)) {
    std::cerr << "error: --workload option was not provided" << std::endl;
    return 1;
  }

  tb::Properties props;
  {
    std::string prop_file;
    if (args.get("properties", prop_file)) {
      props = tb::Properties::Make(prop_file);
    }
  }

  const auto config = tb::Configuration::Make(workload);

  std::string database;
  if (!args.get("database", database)) {
    std::cerr << "error: --database option was not provided" << std::endl;
    return 1;
  }
  try {
    const auto result = ExecuteByName(database, config, props);

    std::string result_output_file;
    if (args.get("result", result_output_file)) {
      std::ofstream fout(result_output_file);
      result.dump(fout);
    } else {
      result.dump(std::cout);
    }
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
  }

  return 0;
}
