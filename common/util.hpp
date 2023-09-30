#pragma once

#include "common.hpp"
#include "Logger.hpp"

namespace bridge
{
namespace detail
{

struct Failure {};

template <typename T>
using CheckedResult = outcome::checked<T, Failure>;

template <typename T>
inline CheckedResult<T> get_value_res(
  const ptree& tree,
  const std::string& key)
{
  try
  {
    return tree.get<T>(key);
  }
  catch(std::exception& e)
  {
    global_logger.Debug("get_value_res()", "error: {}", e.what());
    return outcome::failure(Failure {});
  }
}

inline CheckedResult<std::reference_wrapper<ptree>>
get_child_res(
  ptree& tree,
  const std::string& key)
{
  try
  {
    return {std::reference_wrapper(tree.get_child(key))};
  }
  catch(std::exception& e)
  {
    global_logger.Debug("get_child_res()", "error: {}", e.what());
    return outcome::failure(Failure {});
  }
}

inline auto bind_event_to_this(auto ptr, auto self)
  requires std::is_pointer_v<decltype(self)>
{
  using namespace boost::placeholders;
  return boost::bind(ptr, self, _1);
}

inline std::function<bool(const ptree::value_type&)>
pair_finder(const std::string& client, dpp::snowflake channel)
{
  return
    [=](const ptree::value_type& element)
    {
      global_logger.Debug(
        "pair_finder()::lambda",
        "client: {} channel: {}",
        client,
        static_cast<u64>(channel));

      auto current_client =
        detail::get_value_res<std::string>(element.second, "client");
      auto current_channel =
        detail::get_value_res<u64>(element.second, "channel");

      if (current_client.has_value() && current_client.value() == client &&
          current_channel.has_value() && current_channel.value() == channel)
      {
        return true;
      }
      return false;
    };
}

} // detail
} // bridge
