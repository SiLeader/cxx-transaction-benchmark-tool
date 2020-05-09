//
// Created by cerussite on 5/3/20.
//

#pragma once

#include <algorithm>
#include <chrono>
#include <numeric>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace tb {

class Statistics {
 public:
  using ElapsedTimeType = std::chrono::microseconds;
  using ElapsedTimesType = std::vector<ElapsedTimeType>;
  using ElapsedTImesPerThreadType =
      std::tuple<ElapsedTimesType, ElapsedTimesType>;

 private:
  inline static constexpr std::size_t kSuccessIndex = 0, kErrorIndex = 1;

 private:
  std::string name_;
  std::size_t thread_count_;
  std::vector<ElapsedTImesPerThreadType> elapsed_times_;

 public:
  Statistics(std::string name, std::size_t thread_count,
             std::vector<ElapsedTImesPerThreadType> elapsed_times)
      : name_(std::move(name)),
        thread_count_(thread_count),
        elapsed_times_(std::move(elapsed_times)) {}

 public:
  [[nodiscard]] const std::string& name() const noexcept { return name_; }
  [[nodiscard]] std::size_t threadCount() const noexcept {
    return thread_count_;
  }

  template <std::size_t Index>
  [[nodiscard]] std::vector<std::chrono::microseconds> concat(
      int thread_id = -1) const {
    std::vector<std::chrono::microseconds> uss;

    if (thread_id >= 0) {
      return std::get<Index>(elapsed_times_[thread_id]);
    }

    for (const auto& et : elapsed_times_) {
      uss.insert(std::end(uss), std::begin(std::get<Index>(et)),
                 std::end(std::get<Index>(et)));
    }

    return uss;
  }

  template <std::size_t Index>
  [[nodiscard]] std::chrono::microseconds wholeElapsed(
      int thread_id = -1) const {
    auto concatenated = concat<Index>(thread_id);

    return std::accumulate(std::begin(concatenated), std::end(concatenated),
                           std::chrono::microseconds(0));
  }

  template <std::size_t Index>
  [[nodiscard]] std::size_t wholeCount(int thread_id = -1) const {
    return std::size(concat<Index>(thread_id));
  }

  template <std::size_t Index>
  [[nodiscard]] std::chrono::microseconds average(int thread_id = -1) const {
    const auto whole_elapsed = wholeElapsed<Index>(thread_id);
    const auto whole_count = wholeCount<Index>(thread_id);
    if (whole_count == 0) {
      return std::chrono::microseconds(0);
    }

    return whole_elapsed / whole_count;
  }
  [[nodiscard]] std::chrono::microseconds average(int thread_id = -1) const {
    const auto whole_elapsed = wholeElapsed<kSuccessIndex>(thread_id) +
                               wholeElapsed<kErrorIndex>(thread_id);
    const auto whole_count = wholeCount<kSuccessIndex>(thread_id) +
                             wholeCount<kErrorIndex>(thread_id);
    if (whole_count == 0) {
      return std::chrono::microseconds(0);
    }
    return whole_elapsed / whole_count;
  }

  template <std::size_t Index>
  [[nodiscard]] std::chrono::microseconds max(int thread_id = -1) const {
    auto concatenated = concat<Index>(thread_id);
    if (std::empty(concatenated)) {
      return std::chrono::microseconds(0);
    }

    return *std::max_element(std::begin(concatenated), std::end(concatenated));
  }

  template <std::size_t Index>
  [[nodiscard]] std::chrono::microseconds min(int thread_id = -1) const {
    auto concatenated = concat<Index>(thread_id);
    if (std::empty(concatenated)) {
      return std::chrono::microseconds(0);
    }

    return *std::min_element(std::begin(concatenated), std::end(concatenated));
  }

  template <std::size_t Index>
  [[nodiscard]] std::chrono::microseconds median(int thread_id = -1) const {
    auto concatenated = concat<Index>(thread_id);
    if (std::empty(concatenated)) {
      return std::chrono::microseconds(0);
    }
    std::sort(std::begin(concatenated), std::end(concatenated));
    return concatenated[std::size(concatenated) / 2];
  }

  [[nodiscard]] std::chrono::microseconds median(int thread_id = -1) const {
    auto concatenated = concat<kSuccessIndex>(thread_id);
    {
      auto error = concat<kErrorIndex>(thread_id);
      concatenated.insert(std::end(concatenated), std::begin(error),
                          std::end(error));
    }

    if (std::empty(concatenated)) {
      return std::chrono::microseconds(0);
    }
    std::sort(std::begin(concatenated), std::end(concatenated));
    return concatenated[std::size(concatenated) / 2];
  }

  void dump(std::ostream& os) const {
    const auto success_count = wholeCount<kSuccessIndex>();
    const auto error_count = wholeCount<kErrorIndex>();

    const auto success_average = average<kSuccessIndex>();
    const auto error_average = average<kErrorIndex>();
    const auto whole_average = average();

    const auto success_max = max<kSuccessIndex>();
    const auto error_max = max<kErrorIndex>();

    const auto success_min = min<kSuccessIndex>();
    const auto error_min = min<kErrorIndex>();

    const auto success_median = median<kSuccessIndex>();
    const auto error_median = median<kErrorIndex>();
    const auto whole_median = median();

    os << std::dec;
    os << "name: " << name() << "\n"
       << "threads: " << threadCount() << "\n"
       << "count:\n"
       << "  whole: " << (success_count + error_count) << "\n"
       << "  success: " << success_count << "\n"
       << "  error: " << error_count << "\n"
       << "statistics:\n"
       << "  unit: us\n"
       // average
       << "  average:\n"

       << "    whole: " << whole_average.count() << "\n"
       << "    success: " << success_average.count() << "\n"
       << "    error: " << error_average.count()
       << "\n"
       // max
       << "  max:\n"
       << "    whole: " << std::max(success_max, error_max).count() << "\n"
       << "    success: " << success_max.count() << "\n"
       << "    error: " << error_max.count()
       << "\n"
       // min
       << "  min:\n"
       << "    whole: " << std::min(success_min, error_min).count() << "\n"
       << "    success: " << success_min.count() << "\n"
       << "    error: " << error_min.count()
       << "\n"
       // median
       << "  median:\n"
       << "    whole: " << whole_median.count() << "\n"
       << "    success: " << success_median.count() << "\n"
       << "    error: " << error_median.count() << "\n";
  }

  [[nodiscard]] std::string dump() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
  }
};

}  // namespace tb
