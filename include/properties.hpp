//
// Created by cerussite on 5/8/20.
//

#pragma once

#include <fstream>
#include <istream>
#include <optional>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace tb {

namespace detail {

template <class Integral>
std::optional<Integral> FromString(const std::string& str);

template <>
std::optional<int> FromString<int>(const std::string& str) {
  try {
    return std::stoi(str);
  } catch (const std::exception&) {
    return std::nullopt;
  }
}

}  // namespace detail

class Properties {
 private:
  std::unordered_map<std::string, std::string> values_;

 public:
  Properties() = default;
  explicit Properties(std::unordered_map<std::string, std::string> values)
      : values_(std::move(values)) {}

  Properties(const Properties&) = default;
  Properties(Properties&&) = default;

  Properties& operator=(const Properties&) = default;
  Properties& operator=(Properties&&) = default;

  ~Properties() = default;

 public:
  static Properties Make(std::istream& is) {
    std::unordered_map<std::string, std::string> values;

    while (is) {
      std::string line;
      std::getline(is, line);
      if (line[0] == '#') {
        continue;
      }

      auto pos = line.find('=');
      if (pos == std::string::npos) {
        continue;
      }

      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);

      values[key] = value;
    }

    return Properties{values};
  }

  static Properties Make(const std::string& file_name) {
    std::ifstream fin(file_name);
    return Make(fin);
  }

 public:
  std::optional<std::string> getProperty(const std::string& key) const {
    auto itr = values_.find(key);
    if (itr == std::end(values_)) {
      return std::nullopt;
    }
    return itr->second;
  }

  std::string getProperty(const std::string& key,
                          const std::string& default_value) const {
    return getProperty(key).value_or(default_value);
  }

  template <class IntegralT,
            class = std::enable_if_t<std::is_integral_v<IntegralT>>>
  std::optional<IntegralT> get(const std::string& key) const {
    const auto value = getProperty(key);
    if (!value) {
      return std::nullopt;
    }
    return detail::FromString<IntegralT>(*value);
  }

  template <class IntegralT,
            class = std::enable_if_t<std::is_integral_v<IntegralT>>>
  IntegralT get(const std::string& key, IntegralT default_value) const {
    const auto value = get<IntegralT>(key);
    return value ? *value : default_value;
  }
};

}  // namespace tb
