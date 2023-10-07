#include "Bot.hpp"
#include "common/util.hpp"

using namespace bridge;

std::vector<BotCommandPtr> BotCommand::commands_list_;

auto BotCommand::Create(
  BotChatSession& bot,
  const BotSettingsPtr& settings,
  dpp::slashcommand&& command) -> BotCommandPtr
{

  auto ptr = BotCommandPtr(
    new BotCommand(
      settings,
      std::forward<decltype(command)>(command)));

  bot.bot_.global_command_create(ptr->command_, bot.BindToLogger());
  commands_list_.push_back(ptr);

  return ptr;
}

BotCommand::BotCommand(
  const BotSettingsPtr& settings,
  dpp::slashcommand&& command)
  : settings(settings),
    command_(std::forward<decltype(command)>(command)) {}

auto BotCommand::SetCallable(
  command_callable_t &&callable) -> BotCommand&
{
  callable_ = callable;
  is_set_ = true;

  return *this;
}

void BotCommand::Call(const dpp::slashcommand_t &event)
{
  const std::string &command_name = event.command.get_command_name();

  bool found = false;

  auto command_finder =
    [&command_name, &found, &event]
    (BotCommandPtr &current_cmd)
  {
    Debug(
      "Call::command_finder()",
      "current_cmd: {} other_cmd: {}",
      current_cmd->command_.name, command_name);

    if (current_cmd->command_.name != command_name)
    {
      return;
    }

    found = true;
    if (current_cmd->is_set_)
    {
      current_cmd->callable_(*current_cmd, event);
      return;
    }

    throw std::logic_error(
      fmt::format("Callable is not set for: {}", current_cmd->command_.name));
  };

  std::ranges::for_each(commands_list_, command_finder);

  if (!found)
  {
    throw std::out_of_range(fmt::format("Command not found! {}", command_name));
  }
}

BotChatSession::BotChatSession(
  const std::shared_ptr<ThreadSafeChatRoom> &room)
  : room_(room),
    bot_(TOKEN, dpp::i_default_intents | dpp::i_message_content)
{
  Debug("Constructor", "Application ID: {}", static_cast<u64>(bot_.me.id));

  settings_ = std::make_shared<BotSettings>();

  bot_.on_message_create(
    BindToThis(&BotChatSession::OnMessageCreate, this));
  Debug("Constructor", "bound OnMessageCreate");

  bot_.on_slashcommand(
    BindToThis(&BotChatSession::OnSlashCommand, this));

  bot_.on_ready(BindToThis(&BotChatSession::OnReady, this));
  Debug("Constructor", "bound OnReady");
}

BotChatSession::~BotChatSession() { room_->Leave(shared_from_this()); }

void BotChatSession::Start()
{
  bot_.start(dpp::st_return);
  Debug("Start()", "bot started");

  room_->Join(shared_from_this());
}

void BotChatSession::OnMessageCreate(
  const dpp::message_create_t &event)
{
  if (event.msg.author.id == bot_.me.id)
  {
    // well I pretty much know when I write messages...
    return;
  }

  Debug(
    "OnMessageCreate()", "message: {}, channel: {}",
    event.msg.content,
    static_cast<u64>(event.msg.channel_id));

  auto formatted_msg = MessageFormatter::ConstructJson({
    event.msg.author.global_name,
    event.msg.content,
    "Discord"});

  room_->DeliverMessage(shared_from_this(), formatted_msg);
  Debug("OnMessageCreate()", "delivered message");
}

void BotChatSession::OnSlashCommand(const dpp::slashcommand_t &event)
{
  Debug("OnSlashCommand()", "command: {}", event.command.get_command_name());

  BotCommand::Call(event);

  Debug(
    "OnSlashCommand()",
    "command processed: {}",
    event.command.get_command_name());
}

void BotChatSession::OnReady([[maybe_unused]] const dpp::ready_t &event)
{
  if (dpp::run_once<struct register_global_commands>())
  {
    Debug("OnReady()");
    try
    {
      dpp::slashcommand slashcmd = dpp::slashcommand
        {
          "bindclient",
          "Binds a client to this specific channel",
          bot_.me.id
        }
        .add_option({
          dpp::co_string,
          "client",
          "The client to be bound to this channel",
          true});

      BotCommand::Create(
        *this,
        settings_,
        std::move(slashcmd))
        ->SetCallable(
          [self = shared_from_this()]
          (BotCommand& command, const dpp::slashcommand_t event) {
            self->HandleBindCommand(command, event);
          });

      slashcmd = dpp::slashcommand
        {
          "unbindclient",
          "Unbinds a specific client from this channel",
          bot_.me.id
        }
        .add_option({
          dpp::co_string,
          "client",
          "The client to be unbound from this channel",
          true});

      BotCommand::Create(
        *this,
        settings_,
        std::move(slashcmd))
        ->SetCallable(
          [self = shared_from_this()]
          (BotCommand& command, const dpp::slashcommand_t event) {
            self->HandleUnbindCommand(command, event);
          });

      BotCommand::Create(
        *this,
        settings_,
        {
          "listclients",
          "Lists all clients bound to this channel",
          bot_.me.id
        })
        ->SetCallable(
          [self = shared_from_this()]
          (BotCommand& command, const dpp::slashcommand_t event) {
            self->HandleListBoundCommand(command, event);
          });
    }
    catch (std::exception& e)
    {
      Debug("OnReady()", "error: {}", e.what());
    }
  }
}

void BotChatSession::HandleBindCommand(
  const BotCommand& command,
  const dpp::slashcommand_t& event)
{
  Debug("handle_bind_command()");
  if (dpp::snowflake channel_id = event.command.channel_id)
  {
    std::string client = std::get<std::string>(event.get_parameter("client"));
    if (command.settings->BindClient(client, channel_id))
    {
      event.reply(fmt::format(
        "Successfully bound client `{}` to channel with ID: `{}`",
        client,
        static_cast<u64>(channel_id)));
    }
    else
    {
      event.reply(fmt::format(
        "Client `{}` already bound to this channel!",
        client));
    }
  }
}

void BotChatSession::HandleUnbindCommand(
  const BotCommand& command,
  const dpp::slashcommand_t& event)
{
  if (dpp::snowflake channel_id = event.command.channel_id)
  {
    std::string client = std::get<std::string>(event.get_parameter("client"));
    if (command.settings->UnbindClient(client, channel_id))
    {
      event.reply(fmt::format(
        "Successfully unbound client `{}` from channel with ID: `{}`",
        client,
        static_cast<u64>(channel_id)));
    }
    else
    {
      event.reply(fmt::format(
        "Client `{}` is not bound to channel with ID: `{}`!",
        client,
        static_cast<u64>(channel_id)));
    }
  }
}

void BotChatSession::HandleListBoundCommand(
  const BotCommand& command,
  const dpp::slashcommand_t& event)
{
  if (dpp::snowflake channel_id = event.command.channel_id)
  {
    try
    {
      auto client_view = command.settings->GetClientList(channel_id);
      event.reply(fmt::format("Clients bound to this channel: {}", client_view));
    }
    catch (std::exception& e)
    {
      Debug("HandleListBoundCommand()", "error: {}", e.what());
    }
  }
}

void BotChatSession::DeliverMessage(
  const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  if (participant.get() == this)
  {
    return;
  }

  MessageParser parser(message);
  auto client_result = parser.GetClient();
  if (!client_result.has_value())
  {
    return;
  }

  auto channels_view = settings_->GetChannelList(client_result.value());
  if (channels_view.empty())
  {
    return;
  }

  std::string formatted;
  try
  {
    formatted = fmt::format("{}", parser);
  }
  catch (std::exception& e)
  {
    Debug("DeliverMessage()", "error: {}", e.what());
    return;
  }

  std::ranges::for_each(channels_view, [this, &formatted](u64 channel) {
    bot_.message_create({channel, formatted}, BindToLogger());
    Debug("DeliverMessage()", "Channel Found: {}", channel);
  });
}

auto BotChatSession::BindToLogger() -> logger_cb_t
{
  return BindToLogger(default_cb_t);
}

auto BotChatSession::BindToLogger(
  logger_cb_t &callable) -> logger_cb_t
{
  return [callable](const dpp::confirmation_callback_t &result)
  {
    if (result.is_error())
    {
      Debug("BindToLogger()", "error: {}", result.get_error().message);
      return;
    }

    callable(result);
  };
}
