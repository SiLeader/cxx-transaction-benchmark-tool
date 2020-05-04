//
// Created by cerussite on 5/2/20.
//

#pragma once

#include <atomic>
#include <chrono>
#include <future>
#include <shared_mutex>
#include <tuple>
#include <vector>

#include "configuration.hpp"
#include "statistics.hpp"

#define tb_likely(x) __builtin_expect(!!(x), 1)
#define tb_unlikely(x) __builtin_expect(!!(x), 0)

namespace tb {

template <class Database, class... Args>
class Executor {
 private:
  std::tuple<Args...> args_;
  std::shared_mutex shared_mutex_;
  std::atomic<std::size_t> thread_counter_;

 private:
  class InternalStat {
   private:
    std::vector<std::chrono::microseconds> error_elapsed_times_;
    std::vector<std::chrono::microseconds> elapsed_times_;

   public:
    InternalStat() = default;

    explicit InternalStat(std::size_t reserved_length) {
      elapsed_times_.reserve(reserved_length);
      error_elapsed_times_.reserve(reserved_length);
    }

   public:
    template <class TimePoint>
    void addElapsed(const TimePoint& begin, const TimePoint& end) {
      addElapsed(
          std::chrono::duration_cast<std::chrono::microseconds>(end - begin));
    }

    void addElapsed(std::chrono::microseconds us) {
      elapsed_times_.emplace_back(us);
    }

   public:
    template <class TimePoint>
    void addError(const TimePoint& begin, const TimePoint& end) {
      addError(
          std::chrono::duration_cast<std::chrono::microseconds>(end - begin));
    }

    void addError(std::chrono::microseconds us) {
      error_elapsed_times_.emplace_back(us);
    }

   public:
    template <class TimePoint>
    void addEntry(bool is_success, const TimePoint& begin,
                  const TimePoint& end) {
      if (tb_likely(is_success)) {
        addElapsed(begin, end);
      } else {
        addError(begin, end);
      }
    }

   public:
    Statistics::ElapsedTImesPerThreadType toStatisticsElement() const {
      return {elapsed_times_, error_elapsed_times_};
    }
  };

 public:
  explicit Executor(std::tuple<Args...> args)
      : args_(std::move(args)), thread_counter_(0) {}
  explicit Executor(Args... args) : Executor(std::tuple{args...}) {}

 public:
  Database create() { return std::apply(Database::Make, args_); }

 private:
  InternalStat executeImpl(const tb::Configuration& config) {
    auto transactions = config.createQueries();
    auto db = create();
    InternalStat stat(config.count());

    thread_counter_++;
    shared_mutex_.lock_shared();  // block

    using Clock = std::chrono::system_clock;

    for (const auto& queries : transactions) {
      bool is_success = true;
      auto begin = Clock::now();

      try {
        for (const auto& query : queries) {
          db.execute(query);
        }
      } catch (...) {
        is_success = false;
      }

      auto end = Clock::now();

      stat.addEntry(is_success, begin, end);
    }

    return stat;
  }

 public:
  Statistics execute(const Configuration& config) {
    thread_counter_ = 0;

    std::vector<std::future<InternalStat>> stat_futures;
    stat_futures.reserve(config.threadCount());

    shared_mutex_.lock();

    for (std::size_t i = 0; i < config.threadCount(); ++i) {
      stat_futures.emplace_back(
          std::async(std::launch::async, [&] { return executeImpl(config); }));
    }

    while (thread_counter_ < config.threadCount()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    shared_mutex_.unlock();

    std::vector<InternalStat> iss(std::size(stat_futures));
    std::transform(
        std::begin(stat_futures), std::end(stat_futures), std::begin(iss),
        [](std::future<InternalStat>& future) { return future.get(); });

    std::vector<Statistics::ElapsedTImesPerThreadType> etpts(std::size(iss));
    std::transform(
        std::begin(iss), std::end(iss), std::begin(etpts),
        [](const InternalStat& is) { return is.toStatisticsElement(); });

    return Statistics(config.name(), config.threadCount(), etpts);
  }
};

template <class... Args>
Executor(Args...) -> Executor<Args...>;

template <class... Args>
Executor(std::tuple<Args...>) -> Executor<Args...>;

}  // namespace tb
