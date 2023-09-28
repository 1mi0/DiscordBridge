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
  void DeliverMessage(
    const ChatRoomParticipantPtr &participant,
    const std::string &message);

  void Join(const ChatRoomParticipantPtr &participant);

  void Leave(const ChatRoomParticipantPtr &participant);
};

class ThreadSafeChatRoom
  : protected ChatRoom,
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

  // Spawn the delivery in the io_context
  void Start();

private:
  awaitable<void> Delivery();

public:
  // Deliver a message from any thread
  void DeliverMessageSafe(
    const ChatRoomParticipantPtr &participant,
    const std::string &message);

  // the unsafe versions should only be used from the thread that
  // ThreadSafeChatRoom runs in
  void DeliverMessageUnsafe(
    const ChatRoomParticipantPtr &participant,
    const std::string &message);

private:
  // Just an internal to the class method to deal with message_
  void DeliverMessageUnsafe(const message_ &message);

public:
  // Join ONLY on the main thread
  void JoinUnsafe(const ChatRoomParticipantPtr &participant);

  // Leave ONLY on the main thread
  void LeaveUnsafe(const ChatRoomParticipantPtr &participant);
};

}
