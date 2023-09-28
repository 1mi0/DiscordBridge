#include "BotSettings.hpp"

using namespace bridge;

BotSettings::BotSettings() { LoadClients(); }

BotSettings::~BotSettings() { FlushAll(); }

/*auto BotSettings::GetChannel(const std::string &client)
  -> std::optional<dpp::snowflake>
{
  auto value_array =
    detail::get_child_to_optional(client_to_channel_, "values");

  if (!value_array.has_value())
  {
    return {};
  }

  u64 found = 0;
  for (const auto &[key, value] : value_array.value().get())
  {
    auto current_client_opt =
      detail::get_value_to_optional<std::string>(value, "client");

    if (current_client_opt.has_value() &&
        current_client_opt.value() == client)
    {
      auto channel_opt = detail::get_value_to_optional<u64>(value, "channel");
      if (channel_opt.has_value())
      {
        found = channel_opt.value();
        break;
      }
    }
  }

  if (found == 0)
  {
    return {};
  }

  return found;
}*/

auto BotSettings::BindClient(
  const std::string &client,
  dpp::snowflake channel) -> bool
{
  detail::Defer<true> flush([this] { FlushClients(); });

  ptree obj;
  obj.add("client", client);
  obj.add("channel", static_cast<u64>(channel));

  auto value_array_opt =
    detail::get_child_to_optional(client_to_channel_, "values");

  if (value_array_opt.has_value())
  {
    auto &value_array = value_array_opt.value().get();
    try
    {
      if (CheckClientExists(value_array, client, channel))
      {
        flush.cancel();
        return false;
      }

      PushClient(value_array, obj);
      return true;
    }
    catch (std::exception &e)
    {
      Log("bind_client_to_channel()", "error: {}", e.what());
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
  detail::Defer flush([this] { FlushClients(); });

  auto value_array_opt =
    detail::get_child_to_optional(client_to_channel_, "values");

  if (!value_array_opt.has_value())
  {
    return false;
  }

  auto &value_array = value_array_opt.value().get();

  // erase if is not allowed since value_array's iterators are constant
  auto found =
    std::ranges::find_if(value_array, detail::pair_finder(client, channel));

  if (found == value_array.end())
  {
    return false;
  }

  value_array.erase(found);
  return true;
}

auto BotSettings::CheckClientExists(
  ptree &value_array,
  const std::string &client,
  dpp::snowflake channel) -> bool
{

  bool found =
    std::ranges::find_if(value_array, detail::pair_finder(client, channel)) !=
    value_array.end();

  Log("check_client_to_channel()", "found: {}", found);
  return found;
}

void BotSettings::PushClient(ptree &values_array, ptree &obj)
{
  values_array.push_back(std::make_pair("", obj));
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
  const std::string &file_name) -> std::optional<std::unique_ptr<ptree>>
{
  auto tree = std::make_unique<ptree>();

  std::ifstream stream(file_name);
  if (!stream.is_open())
  {
    return {};
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
    Log("tree_to_file()", "error: {}", e.what());
    return false;
  }

  return true;
}

