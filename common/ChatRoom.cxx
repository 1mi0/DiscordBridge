#include "ChatRoom.hpp"
#include <mutex>

using namespace bridge;

void ChatRoom::DeliverMessage(
  const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  for (auto& current : participants_)
  {
    if (current != participant)
    {
      current->DeliverMessage(participant, message);
    }
  }
}

void ChatRoom::Join(const ChatRoomParticipantPtr &participant)
{
  participants_.insert(participant);
  Debug("Join()", "total: {}", participants_.size());
}

void ChatRoom::Leave(const ChatRoomParticipantPtr &participant)
{
  participants_.erase(participant);
  Debug("Leave()", "total: {}", participants_.size());
}

ThreadSafeChatRoom::ThreadSafeChatRoom(asio::io_context &io)
  : timer_(io)
{
  timer_.expires_at(std::chrono::steady_clock::time_point::max());

  co_spawn(timer_.get_executor(), Delivery(), detached);
}

awaitable<void> ThreadSafeChatRoom::Delivery()
{
  Debug("Delivery()");
  while (true)
  {
    try
    {
      Debug("Delivery()", "iteration start");

      std::deque<message_> copied_msgs;
      {
        boost::mutex::scoped_lock scoped_lock(mutex_);
        copied_msgs = deliver_messages_;
        deliver_messages_.clear();
      }

      Debug("Delivery()", "copied msgs");
      while (!copied_msgs.empty())
      {
        DeliverMessageUnsafe(copied_msgs.front());
        copied_msgs.pop_front();
      }

      boost::system::error_code ec;
      co_await timer_.async_wait(asio::redirect_error(use_awaitable, ec));
    }
    catch (std::exception &e)
    {
      Debug("Delivery()", "caught: {}", e.what());
    }
  }
}

void ThreadSafeChatRoom::DeliverMessage(
  const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  Debug("DeliverMessageSafe()");
  {
    boost::mutex::scoped_lock scoped_lock(mutex_);
    deliver_messages_.push_back({participant, message});
  }

  asio::post(timer_.get_executor(), [self = shared_from_this()] {
    self->Debug("asio::post");

    self->timer_.cancel_one();
  });
  Debug("DeliverMessageSafe()", "posted");
}

void ThreadSafeChatRoom::DeliverMessageUnsafe(const message_ &message)
{
  ChatRoom::DeliverMessage(message.participant, message.content);
}

void ThreadSafeChatRoom::Join(const ChatRoomParticipantPtr &participant)
{
  {
    std::scoped_lock lock(mutex_);
    ChatRoom::Join(participant);
  }
}

void ThreadSafeChatRoom::Leave(const ChatRoomParticipantPtr &participant)
{
  {
    std::scoped_lock lock(mutex_);
    ChatRoom::Leave(participant);
  }
}

