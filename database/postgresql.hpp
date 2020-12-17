//
// Created by cerussite on 11/4/20.
//

#pragma once

#include <libpq-fe.h>

#include <database.hpp>
#include <memory>
#include <properties.hpp>

namespace tb::database {

class PostgreSQL : public Database {
 private:
  PGconn* connection_ = nullptr;
  PGresult* result_ = nullptr;

 public:
  PostgreSQL() = default;
  PostgreSQL(const std::string& host, std::uint16_t port,
             const std::string& database, const std::string& user,
             const std::string& password)
      : connection_(PQsetdbLogin(host.c_str(), std::to_string(port).c_str(),
                                 nullptr, nullptr, database.c_str(),
                                 user.c_str(), password.c_str())) {
    if (PQstatus(connection_) == CONNECTION_BAD) {
      std::string message = "connection cannot be established. ";
      message += PQerrorMessage(connection_);
      throw std::runtime_error(message);
    }
  }

  explicit PostgreSQL(const Properties& props)
      : PostgreSQL(props.getProperty("host", "localhost"),
                   props.get<int>("port", 0), props.getProperty("database", ""),
                   props.getProperty("user", ""),
                   props.getProperty("password", "")) {}

  PostgreSQL(const PostgreSQL&) = delete;
  PostgreSQL(PostgreSQL&&) = default;

  PostgreSQL& operator=(const PostgreSQL&) = delete;
  PostgreSQL& operator=(PostgreSQL&&) = default;

  ~PostgreSQL() override { close(); }

  void close() {
    if (result_) {
      PQclear(result_);
      result_ = nullptr;
    }

    if (connection_) {
      PQfinish(connection_);
      connection_ = nullptr;
    }
  }

  static std::unique_ptr<Database> Make(const Properties& props) {
    return std::make_unique<PostgreSQL>(props);
  }

 public:
  void execute(std::string_view query) override {
    result_ = PQexec(connection_, std::string(query).c_str());
    if (PQresultStatus(result_) != PGRES_COMMAND_OK) {
      std::cout << "exec error: " << PQresultErrorMessage(result_) << std::endl;
      throw std::runtime_error("exec error");
    }
  }
};

}  // namespace tb::database
