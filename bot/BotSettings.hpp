#pragma once

#include "common/common.hpp"
#include "common/MessageFormat.hpp"
#include "common/util.hpp"

namespace bridge
{

using namespace boost::property_tree;

constexpr char BOTSETTINGS_STR[] = "BotSettings";
class BotSettings
  : public Logger<BOTSETTINGS_STR>
{
  ptree client_to_channel_;
  std::unordered_map<dpp::snowflake, ptree> guild_to_settings_;
  // TODO: still no guild settings

public:
  [[nodiscard]] BotSettings();

  ~BotSettings();

private:
  static inline auto opt_filter = [](const auto &client_opt) {
    return client_opt.has_value();
  };

  static inline auto opt_transformer = [](const auto &client_opt) {
    return client_opt.value();
  };

  [[nodiscard]] auto FilterChannels(
    ptree &value_array,
    const std::string& client)
  {
    // This has to be in here because it has a deduced type
    auto client_filter = [client](const ptree::value_type &element) {
      auto client_opt =
        detail::get_value_to_optional<std::string>(element.second, "client");

      return client_opt.has_value() && client_opt.value() == client;
    };

    return value_array | std::ranges::views::filter(client_filter);
  }

public:
  // Get a view of channels corresponding to the client
  [[nodiscard]] auto GetChannelList(const std::string &client)
  {
    // Why? Why is this a thing? Why do I have to supply the Predicate to get
    // the iterator type? Now I'm forced to use auto, and exceptions.
    auto value_array_opt =
      detail::get_child_to_optional(client_to_channel_, "values");

    auto channel_to_opt_transformer = [](const auto &elem) {
      return detail::get_value_to_optional<u64>(elem.second, "channel");
    };

    return FilterChannels(value_array_opt.value().get(), client)
           | std::ranges::views::transform(channel_to_opt_transformer)
           | std::ranges::views::filter(opt_filter)
           | std::ranges::views::transform(opt_transformer);
  }

  // Binds a client to a specific channel and flushes the clients
  auto BindClient(const std::string &client, dpp::snowflake channel) -> bool;

private:
  [[nodiscard]] auto FilterClients(
    ptree &value_array,
    dpp::snowflake channel)
  {
    // This has to be in here because it has a deduced type
    auto channel_filter = [channel](const ptree::value_type &element) {
      auto channel_opt =
        detail::get_value_to_optional<u64>(element.second, "channel");

      return channel_opt.has_value() && channel_opt.value() == channel;
    };

    return value_array | std::ranges::views::filter(channel_filter);
  }

public:
  // Gets a list of all Clients bound to a specific Channel
  [[nodiscard]] auto GetClientList(dpp::snowflake channel)
  {
    auto value_array_opt =
      detail::get_child_to_optional(client_to_channel_, "values");

    auto client_to_opt_transformer = [](const auto &elem) {
      return detail::get_value_to_optional<std::string>(elem.second, "client");
    };

    // Please give me transform_if :pray:
    return FilterClients(value_array_opt.value().get(), channel)
           | std::ranges::views::transform(client_to_opt_transformer)
           | std::ranges::views::filter(opt_filter)
           | std::ranges::views::transform(opt_transformer);
  }

  // Unbinds a client from a channel
  auto UnbindClient(const std::string &client, dpp::snowflake channel) -> bool;

private:
  void PushClient(ptree &values_array, ptree &obj);

  [[nodiscard]] auto CheckClientExists(
    ptree &value_array,
    const std::string &client,
    dpp::snowflake channel) -> bool;

public:
  // Loads guild specific settings
  auto LoadSettings(dpp::snowflake guild) -> bool;

  // Loads all clients bound to any channel
  static constexpr char
  client_to_channel_file_name[] = "client_to_channel.json";
  auto LoadClients() -> bool;

private:
  void FlushClients();

  void FlushAll();

  [[nodiscard]] static auto TreeFromFile(const std::string &file_name)
    -> std::optional<std::unique_ptr<ptree>>;

  static auto TreeToFile(const ptree &tree, const std::string &file_name) -> bool;
};

using BotSettingsPtr = std::shared_ptr<BotSettings>;

} // bridge
