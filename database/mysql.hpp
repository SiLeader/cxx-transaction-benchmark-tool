//
// Created by cerussite on 5/8/20.
//

#pragma once

#include <mysql/mysql.h>

#include <memory>
#include <mutex>
#include <properties.hpp>
#include <shared_mutex>
#include <string>

#include "database.hpp"

namespace tb::database {

class MySQL : public Database {
 private:
  std::unique_ptr<MYSQL> connection_;

 public:
  MySQL() = default;
  MySQL(const std::string& host, std::uint16_t port, const std::string& user,
        const std::string& password)
      : connection_(std::make_unique<MYSQL>()) {
    static std::mutex mtx;
    {
      std::lock_guard lg(mtx);
      mysql_init(connection_.get());
    }

    using namespace std::string_literals;

    const auto con =
        mysql_real_connect(connection_.get(), host.c_str(), user.c_str(),
                           password.c_str(), nullptr, port, nullptr, 0);
    if (con == nullptr) {
      const auto message =
          "MySQL connect error: "s + mysql_error(connection_.get());
      close();
      throw std::runtime_error(message);
    }
  }

  explicit MySQL(const Properties& props)
      : MySQL(props.getProperty("host", "localhost"), props.get<int>("port", 0),
              props.getProperty("user", ""),
              props.getProperty("password", "")) {}

  MySQL(const MySQL&) = delete;
  MySQL(MySQL&&) = default;

  MySQL& operator=(const MySQL&) = delete;
  MySQL& operator=(MySQL&&) = default;

  ~MySQL() override { close(); }

  void close() {
    if (connection_) {
      mysql_close(connection_.get());
      connection_.reset();
    }
  }

  static std::unique_ptr<Database> Make(const Properties& props) {
    return std::make_unique<MySQL>(props);
  }

 public:
  void execute(std::string_view query) override {
    using namespace std::string_literals;

    if (mysql_real_query(connection_.get(), query.data(), query.size()) != 0) {
      const auto message =
          "query execute failed: "s + mysql_error(connection_.get());
      std::cout << message << std::endl;
      throw std::runtime_error(message);
    }
  }
};

}  // namespace tb::database
