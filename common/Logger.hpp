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

// TODO: extend fmt to work with more types
template <
  char const* domain>
class Logger
{
public:
  template <typename... T>
  static void Debug(
    [[maybe_unused]] const std::string& current_domain,
    [[maybe_unused]] fmt::format_string<T...> fmt,
    [[maybe_unused]] T&&... args)
  {
    #ifndef NDEBUG
    Print(current_domain, fmt, std::forward<T>(args)...);
    #endif //NDEBUG
  }

  static void Debug(
    [[maybe_unused]] const std::string& current_domain)
  {
    #ifndef NDEBUG
    Print(current_domain);
    #endif // NDEBUG
  }

  template <typename... T>
  static void Print(
    [[maybe_unused]] const std::string& current_domain,
    [[maybe_unused]] fmt::format_string<T...> fmt,
    [[maybe_unused]] T&&... args)
  {
    const std::string result = fmt::format(
      "[LOG - {}::{}] {}",
      domain,
      current_domain,
      fmt::format(fmt, std::forward<T>(args)...));

    LockedPrint(result);
  }

  static void Print(
    const std::string& current_domain)
  {
    const std::string result = fmt::format(
      "[LOG - {}::{}]",
      domain,
      current_domain);

    LockedPrint(result);
  }

private:
  static void LockedPrint(const std::string& message)
  {
    boost::mutex::scoped_lock lock(detail::logger_lock::mutex);
    fmt::println("{}", message);
  }
};

} // bridge

constexpr char GLOBAL_LOGGER_NAME[] = "global";
constexpr bridge::Logger<GLOBAL_LOGGER_NAME> global_logger;
