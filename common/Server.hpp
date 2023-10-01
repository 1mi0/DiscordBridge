#pragma once

#include "common.hpp"
#include "ChatRoom.hpp"
#include "Logger.hpp"

namespace bridge
{

constexpr char CLIENTCHATSESSION_STR[] = "ClientChatSession";
class ClientChatSession
  : public ChatRoomParticipant,
    public std::enable_shared_from_this<ClientChatSession>,
    private Logger<CLIENTCHATSESSION_STR>
{
  using WebSocket = beast::websocket::stream<asio::ip::tcp::socket>;

  WebSocket socket_;
  asio::steady_timer timer_;
  std::shared_ptr<ThreadSafeChatRoom> room_;
  std::deque<std::string> write_messages_;
public:
  [[nodiscard]] ClientChatSession(
    asio::ip::tcp::socket &&tcp_socket,
    const std::shared_ptr<ThreadSafeChatRoom> &room);

  ~ClientChatSession();

  // Schedule everything needed for this to operate
  awaitable<void> Acceptor();

private:
  awaitable<void> Reader();

  awaitable<void> Writer();

  // This method is called when there's a
  // ChatMessage to be delivered to this client
  void DeliverMessage(
    [[maybe_unused]] const ChatRoomParticipantPtr &participant,
    const std::string &message) override;

public:
  // Use this to gracefully stop the connection with any reason
  void Stop(const beast::websocket::close_reason &reason);
};

constexpr char SERVER_STR[] = "Server";
class Server
  : private Logger<SERVER_STR>
{
  asio::io_context& io_context_;
  std::shared_ptr<ThreadSafeChatRoom> room_;
public:
  [[nodiscard]] Server(
    asio::io_context &io,
    const std::shared_ptr<ThreadSafeChatRoom> &room);

  // Schedule the acceptor
  void Start();

private:
  awaitable<void> DealWithAccepting(asio::ip::tcp::acceptor acceptor);
};

} // bridge
