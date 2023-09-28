#pragma once

#include "common.hpp"
#include "Logger.hpp"

namespace bridge
{
namespace detail
{

using ptree = boost::property_tree::ptree;

template <typename T>
concept IsPointer = std::is_pointer_v<T>;

template <bool cancellable = false>
struct Defer
{
  using Function_t = std::function<void(void)>;
private:
  Function_t fn_;
public:
  Defer(Function_t&& func)
  {
    fn_ = std::forward<decltype(func)>(func);
  }

  ~Defer()
  {
    fn_();
  }
};

template <>
struct Defer<true>
{
  using Function_t = std::function<void(void)>;
private:
  Function_t fn_;
  bool cancelled_ = false;
public:
  Defer(Function_t&& func)
  {
    fn_ = std::forward<decltype(func)>(func);
  }

  ~Defer()
  {
    if (!cancelled_)
    {
      fn_();
    }
  }

  void cancel()
  {
    cancelled_ = true;
  }
};

template <typename T>
inline std::optional<T> get_value_to_optional(
  const ptree& tree,
  const std::string& key)
{
  try
  {
    T result = tree.get<T>(key);
    return result;
  }
  catch(std::exception& e)
  {
    global_logger.Log("get_value_to_optional()", "error: {}", e.what());
  }
  return {};
}

inline std::optional<std::reference_wrapper<ptree>>
get_child_to_optional(
  ptree& tree,
  const std::string& key)
{
  try
  {
    boost::property_tree::ptree& result = tree.get_child(key);
    return {std::reference_wrapper(result)};
  }
  catch(std::exception& e)
  {
    global_logger.Log("get_child_to_optional()", "error: {}", e.what());
  }
  return {};
}

inline auto bind_event_to_this(auto ptr, IsPointer auto self)
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
      global_logger.Log(
        "pair_finder()::lambda",
        "client: {} channel: {}",
        client,
        static_cast<u64>(channel));

      auto current_client =
        detail::get_value_to_optional<std::string>(element.second, "client");
      auto current_channel =
        detail::get_value_to_optional<u64>(element.second, "channel");

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
