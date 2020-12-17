//
// Created by cerussite on 5/9/20.
//

#pragma once

#include <functional>
#include <memory>
#include <postgresql.hpp>
#include <string>

#include "properties.hpp"

// provided databases
#include "mysql.hpp"
#include "stdout.hpp"

#if __has_include(<notrack/database_creator.hpp>)

#include <notrack/database_creator.hpp>

#else

namespace tb::database {

inline std::optional<
    std::function<std::unique_ptr<Database>(const Properties&)> >
GetNoTrackDatabaseCreator(const std::string&) {
  return std::nullopt;
}

}  // namespace tb::database

#endif

namespace tb::database {

inline std::function<std::unique_ptr<Database>(const Properties&)>
GetDatabaseCreator(const std::string& name) {
  if (name == "stdout") {
    return [](const Properties& p) { return StdoutTestDatabase::Make(p); };
  } else if (name == "mysql") {
    return [](const Properties& p) { return MySQL::Make(p); };
  } else if (name == "postgresql") {
    return [](const Properties& p) { return PostgreSQL::Make(p); };
  }
  const auto func = GetNoTrackDatabaseCreator(name);
  if (func) {
    return *func;
  }
  throw std::runtime_error("unknown database name " + name);
}

}  // namespace tb::database
