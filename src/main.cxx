#define COMMON_IMPLEMENT_EXTERNS
#include "common/common.hpp"

#include "common/Logger.hpp"
#include "common/ChatRoom.hpp"
#include "common/Server.hpp"
#include "bot/Bot.hpp"
#include "bot/BotSettings.hpp"
#include "common/MessageFormat.hpp"


i32 main()
{
  accuire_envs();

  asio::io_context io_context;
  auto room = std::make_shared<bridge::ThreadSafeChatRoom>(io_context);
  room->Start();

  // make sure not to pass the bot to another thread, completely self contained
  // if thread safety is needed use asio::post with the io_context in this scope
  // also if you want to just turn it off and just have a simple ChatRoom, just
  // comment the 2 lines below :)
  std::make_shared<bridge::BotChatSession>(room->shared_from_this())->Start();
  global_logger.Print("main()", "Bot Running");

  bridge::Server server(io_context, room->shared_from_this());
  server.Start();
  global_logger.Print("main()", "Server Running Port: {}", PORT);

  return 0;
}
