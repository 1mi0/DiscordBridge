#include "MessageFormat.hpp"

using namespace bridge;

auto MessageFormatter::ConstructJson(const Message &msg) -> std::string
{
  return fmt::format(
    "{{\"author\": \"{}\", \"message\": \"{}\", \"client\": \"{}\"}}",
    msg.author, msg.message, msg.client);
}

MessageParser::MessageParser(const std::string &message)
{
  using namespace boost::property_tree;

  auto stream = std::istringstream(message);
  json_parser::read_json(stream, tree_);
}

