#include <cassandra.h>

#include <database.hpp>
#include <memory>
#include <properties.hpp>
#include <thread>
#include <type_traits>

#define CASS_SHARED_PTR(type, v)                                             \
  std::shared_ptr<std::remove_pointer_t<decltype(v)>>(v, [](decltype(v) t) { \
    if (t) cass_##type##_free(t);                                            \
  })

namespace tb::database {

namespace detail {
struct SessionDeleter {
  void operator()(CassSession* s) const noexcept {
    {
      auto close_future = CASS_SHARED_PTR(future, cass_session_close(s));
      cass_future_wait(close_future.get());
    }
    cass_session_free(s);
  }
};
}  // namespace detail

class YugabyteDB : public Database {
 private:
  std::shared_ptr<CassFuture> connect_future_;
  std::shared_ptr<CassCluster> cluster_;
  std::shared_ptr<CassSession> session_;

 public:
  explicit YugabyteDB(const Properties& props)
      : connect_future_(),
        cluster_(CASS_SHARED_PTR(cluster, cass_cluster_new())),
        session_(std::shared_ptr<CassSession>(cass_session_new(),
                                              detail::SessionDeleter())) {
    cass_cluster_set_contact_points(
        cluster_.get(), props.getProperty("host", "localhost").c_str());
    const auto port_opt = props.get<int>("port");
    if (port_opt) {
      cass_cluster_set_port(cluster_.get(), *port_opt);
    }
    cass_cluster_set_connection_heartbeat_interval(cluster_.get(),
                                                   10 * 60 * 60);

    const auto ks_opt = props.getProperty("keyspace");
    if(!ks_opt) {
      throw std::invalid_argument("'keyspace' property is required");
    }

    connect_future_ = CASS_SHARED_PTR(
        future, cass_session_connect_keyspace(session_.get(), cluster_.get(), ks_opt->c_str()));

    if (cass_future_error_code(connect_future_.get()) != CASS_OK) {
      throw std::runtime_error("cannot connect to YugabyteDB");
    }
  }

  ~YugabyteDB() override {
    connect_future_.reset();
    session_.reset();
    cluster_.reset();
  }

  static std::unique_ptr<Database> Make(const Properties& p) {
    return std::make_unique<YugabyteDB>(p);
  }

 public:
  void execute(std::string_view query) override {
    const auto stmt = CASS_SHARED_PTR(
        statement, cass_statement_new_n(query.data(), query.size(), 0));
    const auto exec_future = CASS_SHARED_PTR(
        future, cass_session_execute(session_.get(), stmt.get()));

    const auto error_code = cass_future_error_code(exec_future.get());
    if (error_code != CASS_OK) {
      const auto message =
          "execute failed: code: " + std::to_string(error_code) + ": " +
          cass_error_desc(error_code);
      throw std::runtime_error(message);
    }
  }
};

}  // namespace tb::database