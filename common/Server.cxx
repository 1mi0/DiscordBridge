#include "Server.hpp"

using namespace bridge;

using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::awaitable;
using boost::asio::ip::tcp;

ClientChatSession::ClientChatSession(
  tcp::socket &&tcp_socket,
  const std::shared_ptr<ThreadSafeChatRoom> &room)
  : socket_(std::move(tcp_socket)), timer_(socket_.get_executor()),
    room_(room)
{
  Log("Constructor");

  timer_.expires_at(std::chrono::steady_clock::time_point::max());
}

ClientChatSession::~ClientChatSession() {
  Stop({beast::websocket::close_code::normal});
}

awaitable<void> ClientChatSession::Start()
{
  Log("Start()");

  room_->JoinUnsafe(shared_from_this());
  Log("Start()", "joined a room");

  co_await socket_.async_accept(use_awaitable);
  Log("Start()", "websocket accepted");

  co_spawn(
      socket_.get_executor(),
      [self = shared_from_this()] { return self->Reader(); }, detached);
  Log("Start()", "spawned receiver");

  co_spawn(
      socket_.get_executor(),
      [self = shared_from_this()] { return self->Writer(); }, detached);
  Log("Start()", "spawned writer");
}

awaitable<void> ClientChatSession::Acceptor()
{
  Log("Acceptor()");

  co_await socket_.async_accept(use_awaitable);
  Log("Acceptor()", "accepted");
}

awaitable<void> ClientChatSession::Reader()
{
  Log("Reader()");

  beast::flat_buffer buffer;
  try
  {
    while (true)
    {
      Log("Reader()", "iteration start");

      co_await socket_.async_read(buffer, use_awaitable);
      auto msg = beast::buffers_to_string(buffer.data());
      Log("Reader()", "read async message: {}", msg);

      room_->DeliverMessageSafe(shared_from_this(), msg);
      Log("Reader()", "delivering message");

      buffer.clear();
      Log("Reader()", "buffer cleared");
    }
  }
  catch (std::exception &e)
  {
    Log("Reader()", "caught: {}", e.what());

    Stop({beast::websocket::close_code::internal_error});
  }
}

awaitable<void> ClientChatSession::Writer()
{
  Log("Writer()");

  try
  {
    while (socket_.is_open())
    {
      Log("Writer()", "iteration start");

      if (write_messages_.empty())
      {
        Log("Writer()", "is empty");

        boost::system::error_code ec;
        co_await timer_.async_wait(asio::redirect_error(use_awaitable, ec));
        Log("Writer()", "done waiting");
      }
      else
      {
        Log("Writer()", "is not empty");

        co_await socket_.async_write(asio::buffer(write_messages_.front()),
                                     use_awaitable);
        write_messages_.pop_front();
        Log("Writer()", "message written");
      }
    }
  }
  catch (std::exception &e)
  {
    Log("Writer()", "caught: {}", e.what());

    Stop({beast::websocket::close_code::internal_error});
  }
}

void ClientChatSession::DeliverMessage(
  [[maybe_unused]] const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  Log("deliver_message()");

  write_messages_.push_back(message);
  timer_.cancel_one();
}

void ClientChatSession::Stop(
  const beast::websocket::close_reason &reason)
{
  Log("Stop()");

  room_->LeaveUnsafe(shared_from_this());
  socket_.close(reason);
  timer_.cancel();
}

Server::Server(
  asio::io_context &io,
   const std::shared_ptr<ThreadSafeChatRoom> &room)
  : io_context_(io),
    room_(room)
{
  Log("Constructor");
}

void Server::Start()
{
  Log("Start()");

  co_spawn(
    io_context_,
    DealWithAccepting({io_context_, {tcp::v4(), PORT}}),
    detached);
  Log("Start()", "spawned deal_with_accepting");

  io_context_.run();
}

awaitable<void> Server::DealWithAccepting(tcp::acceptor acceptor)
{
  Log("DealWithAccepting()");

  while (true)
  {
    Log("DealWithAccepting()", "iteration start");

    co_await std::make_shared<ClientChatSession>(
      co_await acceptor.async_accept(use_awaitable),
      room_)->Start();
    Log("DealWithAccepting()", "started session");
  }
}

