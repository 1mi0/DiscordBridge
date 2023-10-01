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
#include "boost/outcome.hpp"

#include "dpp.h"

#include "fmt/ranges.h"
#include "fmt/core.h"

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

inline void AccuireEnvs()
{
  PORT = std::atoi(std::getenv("BRIDGE_PORT"));
  TOKEN = std::getenv("BRIDGE_BOT_TOKEN");
}

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace outcome = BOOST_OUTCOME_V2_NAMESPACE;
using ptree = boost::property_tree::ptree;

#define COMMON_HEADER_
#endif // COMMON_HEADER_

// using guards in this header, because of the code below
#ifdef COMMON_IMPLEMENT_EXTERNS

u16 PORT;
char* TOKEN;

#undef COMMON_IMPLEMENT_EXTERNS
#endif // COMMON_IMPLEMENT_EXTERNS
