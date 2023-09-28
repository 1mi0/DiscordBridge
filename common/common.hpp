#ifndef COMMON_HEADER_

#include <cstdint>
#include <thread>
#include <deque>
#include <set>
#include <map>
#include <fstream>
#include <ranges>

#define BOOST_ASIO_HAS_CO_AWAIT
#include "boost/bind/bind.hpp"
#include "boost/asio.hpp"
#include "boost/asio/awaitable.hpp"
#include "boost/asio/use_awaitable.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/detached.hpp"
#include "boost/beast.hpp"
#include "boost/thread/mutex.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/algorithm/algorithm.hpp"
#include "boost/algorithm/string/find.hpp"

#include "dpp.h"

#include "fmt/ranges.h"

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

extern u16 PORT;
extern char* TOKEN;

inline void accuire_envs()
{
  PORT = std::atoi(std::getenv("BRIDGE_PORT"));
  TOKEN = std::getenv("BRIDGE_BOT_TOKEN");
}

#define COMMON_HEADER_
#endif // COMMON_HEADER_

// using guards in this header, because of the code below
#ifdef COMMON_IMPLEMENT_EXTERNS

u16 PORT;
char* TOKEN;

#undef COMMON_IMPLEMENT_EXTERNS
#endif // COMMON_IMPLEMENT_EXTERNS