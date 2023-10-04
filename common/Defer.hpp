#pragma once

#include "common.hpp"

namespace bridge
{

template <bool cancellable = false>
struct Defer
{
  using Function_t = std::function<void(void)>;
private:
  Function_t function_;
public:
  Defer(Function_t&& func)
  {
    function_ = std::forward<decltype(func)>(func);
  }

  ~Defer()
  {
    function_();
  }
};

template <>
struct Defer<true>
{
  using Function_t = std::function<void(void)>;
private:
  Function_t function_;
  bool cancelled_ = false;
public:
  Defer(Function_t&& func)
  {
    function_ = std::forward<decltype(func)>(func);
  }

  ~Defer()
  {
    if (!cancelled_)
    {
      function_();
    }
  }

  void Cancel()
  {
    cancelled_ = true;
  }
};

} // bridge
