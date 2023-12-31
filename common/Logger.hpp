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

template <
  char const* domain
>
class Logger
{
public:
  template <typename... T>
  inline static void Debug(
    [[maybe_unused]] const std::string& current_domain,
    [[maybe_unused]] fmt::format_string<T...> fmt,
    [[maybe_unused]] T&&... args)
  {
    #ifndef NDEBUG
    PrintWithPrefix("DEBUG", current_domain, fmt, std::forward<T>(args)...);
    #endif //NDEBUG
  }

  inline static void Debug(
    [[maybe_unused]] const std::string& current_domain)
  {
    #ifndef NDEBUG
    PrintWithPrefix("DEBUG", current_domain);
    #endif // NDEBUG
  }

  template <typename... T>
  inline static void Print(
    [[maybe_unused]] const std::string& current_domain,
    [[maybe_unused]] fmt::format_string<T...> fmt,
    [[maybe_unused]] T&&... args)
  {
    PrintWithPrefix("LOG", current_domain, fmt, std::forward<T>(args)...);
  }

  inline static void Print(
    const std::string& current_domain)
  {
    PrintWithPrefix("LOG", current_domain);
  }

private:
  template <typename... T>
  static void PrintWithPrefix(
    const std::string& print_type,
    const std::string& current_domain)
  {
    const std::string result = fmt::format(
      "[{} - {}::{}]",
      print_type,
      domain,
      current_domain);

    LockedPrint(result);
  }

  template <typename... T>
  static void PrintWithPrefix(
    const std::string& print_type,
    const std::string& current_domain,
    fmt::format_string<T...> fmt,
    T&&... args)
  {
    const std::string result = fmt::format(
      "[{} - {}::{}] {}",
      print_type,
      domain,
      current_domain,
      fmt::format(fmt, std::forward<T>(args)...));

    LockedPrint(result);
  }

  static void LockedPrint(const std::string& message)
  {
    boost::mutex::scoped_lock lock(detail::logger_lock::mutex);
    fmt::println("{}", message);
  }
};

} // bridge

constexpr char GLOBAL_LOGGER_NAME[] = "global";
constexpr bridge::Logger<GLOBAL_LOGGER_NAME> global_logger;
