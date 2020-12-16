//
// Created by cerussite on 4/26/20.
//

#pragma once

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <string>
#include <vector>

#include "../src/template_engine.hpp"

namespace tb {

class Configuration {
 private:
  std::string name_;
  std::string begin_, commit_;
  std::vector<std::string> queries_;
  std::size_t count_, thread_count_;
  bool enable_transaction_;

 private:
  Configuration(std::string name, std::string begin, std::string commit,
                std::vector<std::string> queries, std::size_t count,
                std::size_t thread_count, bool enable_transaction)
      : name_(std::move(name)),
        begin_(std::move(begin)),
        commit_(std::move(commit)),
        queries_(std::move(queries)),
        count_(count),
        thread_count_(thread_count),
        enable_transaction_(enable_transaction) {}

 public:
  static Configuration Make(std::string begin, std::string commit,
                            std::istream& is, bool enable_transaction = true) {
    auto config = YAML::Load(is);
    if (config.IsNull()) {
      throw std::runtime_error("configuration file cannot be read");
    }

    auto name = config["name"].as<std::string>();

    auto count_node = config["count"];
    const auto count = count_node.as<int>();

    auto thread_count_node = config["threads"];
    const auto thread_count = thread_count_node.as<int>();

    auto transaction_node = config["transaction"];

    std::vector<std::string> queries;
    auto queries_node = transaction_node["queries"];
    queries.reserve(std::size(queries_node));
    for (const auto& query_node : queries_node) {
      queries.emplace_back(query_node.as<std::string>());
    }

    return Configuration(std::move(name), std::move(begin), std::move(commit),
                         std::move(queries), count, thread_count,
                         enable_transaction);
  }

  static Configuration Make(std::string begin, std::string commit,
                            const std::string& config_file,
                            bool enable_transaction = true) {
    std::ifstream fin(config_file);
    if (!fin) {
      throw std::runtime_error("cannot open configuration file");
    }
    return Make(std::move(begin), std::move(commit), fin, enable_transaction);
  }

 public:
  [[nodiscard]] const std::string& name() const noexcept { return name_; }

  void threadCount(std::size_t count) noexcept { thread_count_ = count; }

  [[nodiscard]] std::size_t threadCount() const noexcept {
    return thread_count_;
  }

  [[nodiscard]] const std::vector<std::string>& queries() const noexcept {
    return queries_;
  }

  [[nodiscard]] std::size_t count() const noexcept { return count_; }

  [[nodiscard]] std::vector<std::vector<std::string>> createQueries() const {
    std::vector<std::vector<std::string>> whole_queries;
    whole_queries.reserve(count());

    const auto& original_queries = queries();

    for (std::size_t i = 0; i < count(); ++i) {
      std::vector<std::string> queries_each_transaction;
      queries_each_transaction.reserve(2 + std::size(original_queries));

      if (enable_transaction_) {
        queries_each_transaction.emplace_back(begin_);
      }
      for (const auto& query : original_queries) {
        queries_each_transaction.emplace_back(CreateString(query));
      }
      if (enable_transaction_) {
        queries_each_transaction.emplace_back(commit_);
      }
      whole_queries.emplace_back(queries_each_transaction);
    }

    return whole_queries;
  }

  [[nodiscard]] std::vector<std::string> createWholeQueries() const {
    std::vector<std::string> queries;

    for (const auto& query_in_transaction : createQueries()) {
      queries.insert(std::end(queries), std::begin(query_in_transaction),
                     std::end(query_in_transaction));
    }
    return queries;
  }
};

}  // namespace tb
