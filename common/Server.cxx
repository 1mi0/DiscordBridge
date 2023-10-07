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
  Debug("Constructor");

  timer_.expires_at(std::chrono::steady_clock::time_point::max());
}

ClientChatSession::~ClientChatSession()
{
  //Stop({beast::websocket::close_code::normal});
}

awaitable<void> ClientChatSession::Acceptor()
{
  Debug("Acceptor()");

  co_await socket_.async_accept(use_awaitable);
  Debug("Acceptor()", "accepted");

  room_->Join(shared_from_this());

  co_spawn(
      socket_.get_executor(),
      [self = shared_from_this()] { return self->Reader(); }, detached);
  Debug("Acceptor()", "spawned receiver");

  co_spawn(
      socket_.get_executor(),
      [self = shared_from_this()] { return self->Writer(); }, detached);
  Debug("Acceptor()", "spawned writer");
}

awaitable<void> ClientChatSession::Reader()
{
  Debug("Reader()");

  beast::flat_buffer buffer;
  try
  {
    while (true)
    {
      Debug("Reader()", "iteration start");

      co_await socket_.async_read(buffer, use_awaitable);
      auto msg = beast::buffers_to_string(buffer.data());
      Debug("Reader()", "read async message: {}", msg);

      room_->DeliverMessage(shared_from_this(), msg);
      Debug("Reader()", "delivering message");

      buffer.clear();
      Debug("Reader()", "buffer cleared");
    }
  }
  catch (std::exception &e)
  {
    Debug("Reader()", "caught: {}", e.what());

    Stop({beast::websocket::close_code::internal_error});
  }
}

awaitable<void> ClientChatSession::Writer()
{
  Debug("Writer()");

  try
  {
    while (socket_.is_open())
    {
      Debug("Writer()", "iteration start");

      if (write_messages_.empty())
      {
        Debug("Writer()", "is empty");

        boost::system::error_code ec;
        co_await timer_.async_wait(asio::redirect_error(use_awaitable, ec));
        Debug("Writer()", "done waiting");
      }
      else
      {
        Debug("Writer()", "is not empty");

        co_await socket_.async_write(asio::buffer(write_messages_.front()),
                                     use_awaitable);
        write_messages_.pop_front();
        Debug("Writer()", "message written");
      }
    }
  }
  catch (std::exception &e)
  {
    Debug("Writer()", "caught: {}", e.what());

    Stop({beast::websocket::close_code::internal_error});
  }
}

void ClientChatSession::DeliverMessage(
  [[maybe_unused]] const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  Debug("deliver_message()");

  write_messages_.push_back(message);
  timer_.cancel_one();
}

void ClientChatSession::Stop(
  const beast::websocket::close_reason &reason)
{
  Debug("Stop()");

  room_->Leave(shared_from_this());
  socket_.close(reason);
  timer_.cancel();
}

Server::Server(
  asio::io_context &io,
   const std::shared_ptr<ThreadSafeChatRoom> &room)
  : io_context_(io),
    room_(room)
{
  Debug("Constructor");

  co_spawn(
    io_context_,
    DealWithAccepting({io_context_, {tcp::v4(), PORT}}),
    detached);
  Debug("Constructor", "spawned deal_with_accepting");

  Print("Constructor", "Server Running on Port: {}", PORT);
}

awaitable<void> Server::DealWithAccepting(tcp::acceptor acceptor)
{
  Debug("DealWithAccepting()");

  while (true)
  {
    Debug("DealWithAccepting()", "iteration start");

    auto tcp_socket = co_await acceptor.async_accept(use_awaitable);

    std::shared_ptr<ClientChatSession> session =
      std::make_shared<ClientChatSession>(
        std::move(tcp_socket),
        room_);

    co_spawn(
      io_context_,
      [self = session] { return self->Acceptor(); },
      detached);
    Debug("DealWithAccepting()", "started session");
  }
}

