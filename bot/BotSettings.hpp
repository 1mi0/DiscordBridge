#pragma once

#include "common/common.hpp"
#include "common/MessageFormat.hpp"
#include "common/util.hpp"
#include <bits/ranges_util.h>
#include <ranges>
#include <variant>

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
  template <typename TData, typename TOther>
    requires std::same_as<TData, std::string> || std::same_as<TData, u64>
  static auto FilterUnwrap(const ptree& value_array, TData data)
  {
    std::string checking_for, other;

    if constexpr (std::same_as<TData, u64>)
    {
      checking_for = "channel";
      other = "client";
    }
    else
    {
      checking_for = "client";
      other = "channel";
    }

    auto data_filter = [checking_for, other, data](auto &pair) {
      auto result = detail::TreeGetValue<TData>(pair.second, checking_for);
      if (!result.has_value() || result.value() != data)
      {
        return false;
      }
      return detail::TreeGetValue<TOther>(pair.second, other).has_value();
    };

    auto unwrap_transform = [other](auto &pair) {
      return detail::TreeGetValue<TOther>(pair.second, other).value();
    };

    namespace stdviews = std::ranges::views;
    return value_array
           | stdviews::filter(data_filter)
           | stdviews::transform(unwrap_transform);
  }

public:
  // Get a view of channels corresponding to the client
  [[nodiscard]] auto GetChannelList(const std::string &client)
  {
    // Why? Why is this a thing? Why do I have to supply the Predicate to get
    // the iterator type? Now I'm forced to use auto, and exceptions.
    auto result =
      detail::TreeGetChild(client_to_channel_, "values");

    return FilterUnwrap<std::string, u64>(result.value(), client);
  }

  // Gets a list of all Clients bound to a specific Channel
  [[nodiscard]] auto GetClientList(dpp::snowflake channel)
  {
    auto result =
      detail::TreeGetChild(client_to_channel_, "values");

    // Please give me transform_if :pray:
    return FilterUnwrap<u64, std::string>(
      result.value(),
      static_cast<u64>(channel));
  }

  // Binds a client to a specific channel and flushes the clients
  auto BindClient(const std::string &client, dpp::snowflake channel) -> bool;

  // Unbinds a client from a channel
  auto UnbindClient(const std::string &client, dpp::snowflake channel) -> bool;

  // Loads guild specific settings
  auto LoadSettings(dpp::snowflake guild) -> bool;

  // Loads all clients bound to any channel
  static constexpr char
  client_to_channel_file_name[] = "client_to_channel.json";
  auto LoadClients() -> bool;

private:
  // Flushes all the clients to the file
  void FlushClients();

  // Flushes everything, including guild settings to the appropriate files
  void FlushAll();

  // Adds a client to the ptree provided
  static void PushClient(ptree &value_array, const ptree &obj);

  // Checks whether a pair exists in the ptree provided
  [[nodiscard]] static auto CheckClientExists(
    const ptree &value_array,
    const std::string &client,
    dpp::snowflake channel) -> bool;


  static std::function<bool(const ptree::value_type&)>
  PairFinder(const std::string& client, dpp::snowflake channel)
  {
    return
      [=](const ptree::value_type& element)
      {
        global_logger.Debug(
          "pair_finder()::lambda",
          "client: {} channel: {}",
          client,
          static_cast<u64>(channel));

        auto current_client =
          detail::TreeGetValue<std::string>(element.second, "client");
        auto current_channel =
          detail::TreeGetValue<u64>(element.second, "channel");

        if (current_client.has_value() && current_client.value() == client &&
            current_channel.has_value() && current_channel.value() == channel)
        {
          return true;
        }
        return false;
      };
  }

  // Loads a Tree from a File
  [[nodiscard]] static auto TreeFromFile(const std::string &file_name)
    -> outcome::checked<std::unique_ptr<ptree>, std::string>;

  // Stores a Tree into a File
  static auto TreeToFile(const ptree &tree, const std::string &file_name) -> bool;
};

using BotSettingsPtr = std::shared_ptr<BotSettings>;

} // bridge
