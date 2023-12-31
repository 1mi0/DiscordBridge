#include "BotSettings.hpp"
#include "Defer.hpp"

using namespace bridge;

BotSettings::BotSettings() { LoadClients(); }

BotSettings::~BotSettings() { FlushAll(); }

auto BotSettings::BindClient(
  const std::string &client,
  dpp::snowflake channel) -> bool
{
  Defer<true> flush([this] { FlushClients(); });

  ptree obj;
  obj.add("client", client);
  obj.add("channel", static_cast<u64>(channel));

  auto result =
    detail::TreeGetChild(client_to_channel_, "values");

  if (result.has_value())
  {
    auto &value_array = result.value().get();
    try
    {
      if (CheckClientExists(value_array, client, channel))
      {
        flush.Cancel();
        return false;
      }

      PushClient(value_array, obj);
      return true;
    }
    catch (std::exception &e)
    {
      Debug("bind_client_to_channel()", "error: {}", e.what());
      // value not found is a normal thing, nothing really exceptional
    }
  }

  ptree array;
  array.push_back(std::make_pair("", obj));
  client_to_channel_.add_child("values", array);

  return true;
}

auto BotSettings::UnbindClient(
  const std::string &client,
  dpp::snowflake channel) -> bool
{
  auto result =
    detail::TreeGetChild(client_to_channel_, "values");

  if (!result.has_value())
  {
    return false;
  }

  auto &value_array = result.value().get();

  // erase if is not allowed since value_array's iterators are constant
  auto found =
    std::ranges::find_if(value_array, PairFinder(client, channel));

  if (found == value_array.end())
  {
    return false;
  }

  value_array.erase(found);
  FlushClients();
  return true;
}

auto BotSettings::CheckClientExists(
  const ptree &value_array,
  const std::string &client,
  dpp::snowflake channel) -> bool
{

  bool found =
    std::ranges::find_if(value_array, PairFinder(client, channel)) !=
    value_array.end();

  Debug("check_client_to_channel()", "found: {}", found);
  return found;
}

void BotSettings::PushClient(ptree &value_array, const ptree &obj)
{
  value_array.push_back(std::make_pair("", obj));
}

auto BotSettings::LoadSettings(dpp::snowflake guild) -> bool
{
  std::string file_name =
    fmt::format("settings_{}.json", static_cast<u64>(guild));

  auto settings = TreeFromFile(file_name);

  if (!settings.has_value())
  {
    guild_to_settings_[guild] = {};
    return false;
  }

  guild_to_settings_[guild] = std::move(*settings.value());
  return true;
}

auto BotSettings::LoadClients() -> bool
{
  auto tree = TreeFromFile(client_to_channel_file_name);
  if (!tree.has_value())
  {
    return false;
  }

  client_to_channel_ = std::move(*tree.value());
  return true;
}

void BotSettings::FlushClients()
{
  TreeToFile(client_to_channel_, client_to_channel_file_name);
}

void BotSettings::FlushAll()
{
  FlushClients();
  for (const auto &[key, value] : guild_to_settings_)
  {
    std::string file_name =
      fmt::format("settings_{}.json", static_cast<u64>(key));

    TreeToFile(value, file_name);
  }
}

auto BotSettings::TreeFromFile(
  const std::string &file_name) 
  -> outcome::checked<std::unique_ptr<ptree>, std::string>
{
  auto tree = std::make_unique<ptree>();

  std::ifstream stream(file_name);
  if (!stream.is_open())
  {
    return outcome::failure("Stream didn't open!");
  }

  read_json(stream, *tree);

  return tree;
}

auto BotSettings::TreeToFile(
  const ptree &tree,
  const std::string &file_name) -> bool
{
  try
  {
    write_json(file_name, tree);
  }
  catch (std::exception &e)
  {
    Debug("tree_to_file()", "error: {}", e.what());
    return false;
  }

  return true;
}

