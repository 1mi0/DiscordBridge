#pragma once

#include "common/common.hpp"
#include "common/ChatRoom.hpp"
#include "BotSettings.hpp"

namespace bridge
{

class BotCommand;

constexpr char BOTCHATSESSION_STR[] = "BotChatSession";
class BotChatSession final
  : public ChatRoomParticipant,
    public std::enable_shared_from_this<BotChatSession>,
    private Logger<BOTCHATSESSION_STR>
{

  std::shared_ptr<ThreadSafeChatRoom> room_;
  dpp::cluster bot_;
  BotSettingsPtr settings_;

  // this relation is used so the Command can register itself
  friend class BotCommand;

  static auto BindToThis(auto ptr, auto self)
    requires std::is_pointer_v<decltype(self)>
  {
    using namespace boost::placeholders;
    return boost::bind(ptr, self, _1);
  }
public:
  [[nodiscard]] BotChatSession(
    const std::shared_ptr<ThreadSafeChatRoom> &room);

  ~BotChatSession();

  // Run the bot and join the ChatRoom
  void Start();

private:
  void OnMessageCreate(const dpp::message_create_t &event);

  void OnSlashCommand(const dpp::slashcommand_t &event);

  void OnReady([[maybe_unused]] const dpp::ready_t &event);

  void HandleBindCommand(
    const BotCommand& command,
    const dpp::slashcommand_t& event);

  void HandleUnbindCommand(
    const BotCommand& command,
    const dpp::slashcommand_t& event);

  void HandleListBoundCommand(
    const BotCommand& command,
    const dpp::slashcommand_t& event);

  void DeliverMessage(
    const ChatRoomParticipantPtr &participant,
    const std::string &message) override;

  // The code below is used to bind to any DPP event
  using logger_cb_t =
    std::function<void(const dpp::confirmation_callback_t&)>;

  static inline logger_cb_t default_cb_t
    = [](const dpp::confirmation_callback_t&) {};

  static auto BindToLogger() -> logger_cb_t;

  static auto BindToLogger(logger_cb_t &callable) -> logger_cb_t;
};

using BotChatSessionPtr = std::shared_ptr<BotChatSession>;
using BotCommandPtr = std::shared_ptr<BotCommand>;

constexpr char BOTCOMMAND_STR[] = "BotCommand";
class BotCommand final
  : private Logger<BOTCOMMAND_STR>,
    public std::enable_shared_from_this<BotCommand>
{
  static std::vector<BotCommandPtr> commands_list_;

  using command_callable_t =
    std::function<void(BotCommand&, const dpp::slashcommand_t&)>;

public:
  BotSettingsPtr settings;

private:
  dpp::slashcommand command_;
  command_callable_t callable_;
  bool is_set_ = false;

  [[nodiscard]] BotCommand(
    const BotSettingsPtr& settings,
    dpp::slashcommand&& command);

  void InitializeCommand(BotChatSession& bot);

public:
  auto SetCallable(command_callable_t &&callable) -> BotCommand&;

  static auto Create(
    BotChatSession& bot,
    const BotSettingsPtr& settings,
    dpp::slashcommand&& command) -> BotCommandPtr;

  static void Call(const dpp::slashcommand_t &event);
};

} // bridge
