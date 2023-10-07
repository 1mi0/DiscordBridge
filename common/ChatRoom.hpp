#pragma once

#include "common.hpp"
#include "Logger.hpp"

using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;
using boost::asio::awaitable;
namespace asio = boost::asio;

namespace bridge
{

class ChatRoomParticipant
{
public:
  virtual ~ChatRoomParticipant() {}

protected:
  friend class ChatRoom;
  virtual void DeliverMessage(
    const std::shared_ptr<ChatRoomParticipant>& participant,
    const std::string& message) = 0;
};

using ChatRoomParticipantPtr = std::shared_ptr<ChatRoomParticipant>;

constexpr char CHATROOM_STR[] = "ChatRoom";
class ChatRoom
  : protected Logger<CHATROOM_STR>
{
  std::set<ChatRoomParticipantPtr> participants_;

public:
  virtual void DeliverMessage(
    const ChatRoomParticipantPtr &participant,
    const std::string &message);

  virtual ~ChatRoom() {}

  virtual void Join(const ChatRoomParticipantPtr &participant);

  virtual void Leave(const ChatRoomParticipantPtr &participant);
};

class ThreadSafeChatRoom
  : public ChatRoom,
    public std::enable_shared_from_this<ThreadSafeChatRoom>
{
  struct message_
  {
    ChatRoomParticipantPtr participant;
    std::string content;
  };

  std::deque<message_> deliver_messages_;
  asio::steady_timer timer_;
  boost::mutex mutex_;
public:
  [[nodiscard]] ThreadSafeChatRoom(asio::io_context &io);

private:
  awaitable<void> Delivery();

public:
  // Deliver a message from any thread
  void DeliverMessage(
    const ChatRoomParticipantPtr &participant,
    const std::string &message) override;

private:
  // Just forwards ThreadSafeChatRoom::message_ to the ChatRoom baseclass
  void DeliverMessageUnsafe(const message_ &message);

public:
  void Join(const ChatRoomParticipantPtr &participant) override;

  void Leave(const ChatRoomParticipantPtr &participant) override;
};

}
