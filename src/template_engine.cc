//
// Created by cerussite on 5/1/20.
//

#include "template_engine.hpp"

#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <random>
#include <string>
#include <variant>
#include <vector>

namespace {

template <class Iterator>
struct TemplateEngine
    : public boost::spirit::qi::grammar<Iterator, tb::te::Statement()> {
  template <class T>
  using Rule = boost::spirit::qi::rule<Iterator, T>;

  boost::spirit::qi::rule<Iterator, tb::te::Id()> identifier;
  boost::spirit::qi::rule<Iterator, tb::te::Value()> value;
  boost::spirit::qi::rule<Iterator, std::string()> string_value;
  boost::spirit::qi::rule<Iterator, tb::te::RawString()> raw_string;
  boost::spirit::qi::rule<Iterator, tb::te::Statement()> statement;

  Rule<tb::te::Function()> function;
  Rule<tb::te::Expression()> expression;
  // Rule<tb::te::Statement()> statement;
  Rule<std::vector<tb::te::Valuable>()> arguments;
  Rule<tb::te::Valuable()> valuable;

  TemplateEngine() : TemplateEngine::base_type(statement) {
    namespace qi = boost::spirit::qi;
    using boost::phoenix::bind;
    using boost::phoenix::push_back;
    using qi::_1;
    using qi::_val;
    using qi::space;
    using namespace tb::te;

    statement =
        +(expression[push_back(_val, _1)] | raw_string[push_back(_val, _1)]);
    raw_string = (qi::print)[_val = _1];
    expression = "{{" >> *space >> (function[_val = _1] |
                                    identifier[_val = _1] | value[_val = _1]) >>
                 *space >> "}}";
    function = identifier[bind(&Function::name, _val) = _1] >> *space >> "(" >>
               *space >> -arguments[bind(&Function::args, _val) = _1] >>
               *space >> ")";
    arguments = -(valuable[push_back(_val, _1)] >> *space >>
                  *("," >> *space >> valuable[push_back(_val, _1)]) >> *space);
    identifier = qi::alpha[push_back(_val, _1)] >>
                 *((qi::alnum | qi::char_('_'))[push_back(_val, _1)]);
    value = qi::long_long[_val = _1] | string_value[_val = _1];
    string_value =
        qi::char_('"') >> *qi::print[push_back(_val, _1)] >> qi::char_('"');
    valuable = value[_val = _1] | identifier[_val = _1];
  }
};

struct ValueVisitor {
  std::string operator()(const std::string& str) { return str; }
  std::string operator()(std::int_fast64_t num) { return std::to_string(num); }
};

struct ExpressionVisitor {
  ValueVisitor value_visitor;
  tb::te::FunctionContainer container;

  std::string operator()(const tb::te::Function& func) {
    return this->operator()(container[func.name](func.args));
  }

  std::string operator()(const tb::te::Value& val) {
    return std::visit(value_visitor, val);
  }
};

struct StatementFragmentVisitor {
  ExpressionVisitor expression_visitor;

  std::string operator()(const tb::te::RawString& raw_string) {
    return std::string() + raw_string;
  }

  std::string operator()(const tb::te::Expression& exp) {
    return std::visit(expression_visitor, exp);
  }
};

namespace functions {

tb::te::Value random_string(const std::vector<tb::te::Value>& args) {
  static constexpr std::string_view kChars =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static std::mt19937_64 mt(std::random_device{}());
  static std::uniform_int_distribution<std::int_fast64_t> rand(
      0, std::size(kChars) - 1);

  auto length = std::get<tb::te::kNumberIndex>(args[0]);
  std::string str(length, 0);
  std::generate(std::begin(str), std::end(str),
                [] { return kChars[rand(mt)]; });
  return str;
}

tb::te::Value random_number(const std::vector<tb::te::Value>& args) {
  static std::mt19937_64 mt(std::random_device{}());

  std::int_fast64_t min = std::numeric_limits<std::int_fast64_t>::min(),
                    max = std::numeric_limits<std::int_fast64_t>::max();
  switch (std::size(args)) {
    case 1:
      max = std::get<tb::te::kNumberIndex>(args[0]);
      break;
    case 2:
      min = std::get<tb::te::kNumberIndex>(args[0]);
      max = std::get<tb::te::kNumberIndex>(args[1]);
      break;
  }
  std::uniform_int_distribution<std::int_fast64_t> rand(min, max);

  return rand(mt);
}

}  // namespace functions

}  // namespace

namespace tb {

std::string CreateString(
    const std::string& template_string,
    const std::vector<
        std::pair<std::string, tb::te::FunctionContainer::FunctionType>>&
        functions) {
  auto itr = std::begin(template_string);
  TemplateEngine<std::string::const_iterator> engine_parser;

  tb::te::Statement stmt;
  auto success = boost::spirit::qi::phrase_parse(
      itr, std::end(template_string), engine_parser, boost::spirit::qi::cntrl,
      stmt);
  if (!success) {
    return template_string;
  }

  tb::te::FunctionContainer function_container;
  function_container.addAll({
      {"random_string", functions::random_string},
      {"random_number", functions::random_number},
  });
  function_container.addAll(functions);

  StatementFragmentVisitor stmt_visitor;
  stmt_visitor.expression_visitor.container = function_container;

  std::string result_string;
  for (const auto& stmt_fragment : stmt) {
    result_string += std::visit(stmt_visitor, stmt_fragment);
  }
  return result_string;
}

}  // namespace tb
