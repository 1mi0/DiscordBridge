#pragma once

#include "common.hpp"

namespace bridge
{

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

} // bridge
