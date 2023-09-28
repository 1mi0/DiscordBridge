#pragma once

#include "common.hpp"
#include "Logger.hpp"
#include "util.hpp"

namespace bridge
{

class MessageFormatter
{
public:
  struct [[nodiscard]] Message
  {
    std::string author;
    std::string message;
    std::string client;
  };

  [[nodiscard]] static auto ConstructJson(const Message &msg) -> std::string;
};

constexpr char MESSAGEPARSER_STR[] = "MessageParser";
class MessageParser
  : private Logger<MESSAGEPARSER_STR>
{
  boost::property_tree::ptree tree_;
public:
  MessageParser(const std::string &message);

  std::function<std::optional<std::string>()> GetContent = [this]() {
    return detail::get_value_to_optional<std::string>(tree_, "message");
  };

  std::function<std::optional<std::string>()> GetAuthor = [this]() {
    return detail::get_value_to_optional<std::string>(tree_, "author");
  };

  std::function<std::optional<std::string>()> GetClient = [this]() {
    return detail::get_value_to_optional<std::string>(tree_, "client");
  };
};

}
