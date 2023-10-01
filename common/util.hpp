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
inline CheckedResult<T> TreeGetValue(
  const ptree& tree,
  const std::string& key)
{
  try
  {
    return tree.get<T>(key);
  }
  catch(std::exception& e)
  {
    global_logger.Debug("TreeGetValue()", "error: {}", e.what());
    return outcome::failure(Failure {});
  }
}

inline CheckedResult<std::reference_wrapper<ptree>>
TreeGetChild(
  ptree& tree,
  const std::string& key)
{
  try
  {
    return {std::reference_wrapper(tree.get_child(key))};
  }
  catch(std::exception& e)
  {
    global_logger.Debug("TreeGetChild()", "error: {}", e.what());
    return outcome::failure(Failure {});
  }
}

} // detail
} // bridge
