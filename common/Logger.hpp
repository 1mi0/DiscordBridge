#pragma once

#include "common.hpp"

namespace bridge
{
namespace detail
{

struct logger_lock
{
  static inline boost::mutex mutex;
};

} // detail

// TODO: turn this off in production
// maybe also rename to Debug and Log?
// TODO: extend fmt to work with more types
template <
  char const* domain>
class Logger
{
public:
  template <typename... T>
  static void Log(
    const std::string& current_domain,
    fmt::format_string<T...> fmt,
    T&&... args)
  {
    const std::string result = fmt::format(
      "[LOG - {}::{}] {}",
      domain,
      current_domain,
      fmt::format(fmt, std::forward<T>(args)...));

    {
      boost::mutex::scoped_lock scoped_lock(detail::logger_lock::mutex);
      fmt::println("{}", result);
    }
  }

  static void Log(
    const std::string& current_domain)
  {
    const std::string result = fmt::format(
      "[LOG - {}::{}]",
      domain,
      current_domain);

    {
      boost::mutex::scoped_lock scoped_lock(detail::logger_lock::mutex);
      fmt::println("{}", result);
    }
  }
};

} // bridge

constexpr char GLOBAL_LOGGER_NAME[] = "global";
constexpr bridge::Logger<GLOBAL_LOGGER_NAME> global_logger;
