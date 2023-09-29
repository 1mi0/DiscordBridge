#include "ChatRoom.hpp"

using namespace bridge;

void ChatRoom::DeliverMessage(
  const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  for (auto current_participant : participants_)
  {
    current_participant->DeliverMessage(participant, message);
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
}

void ThreadSafeChatRoom::Start()
{
  Debug("Start()");

  co_spawn(timer_.get_executor(), Delivery(), detached);
  Debug("Start()", "spawned delivery");
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

void ThreadSafeChatRoom::DeliverMessageSafe(
  const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  Debug("DeliverMessageSafe()");
  {
    boost::mutex::scoped_lock scoped_lock(mutex_);
    deliver_messages_.push_back({participant, message});
  }

  asio::post( timer_.get_executor(), [self = shared_from_this()] {
    self->Debug("asio::post");

    self->timer_.cancel_one();
  });
  Debug("DeliverMessageSafe()", "posted");
}

void ThreadSafeChatRoom::DeliverMessageUnsafe(
  const ChatRoomParticipantPtr &participant,
  const std::string &message)
{
  this->DeliverMessage(participant, message);
}

void ThreadSafeChatRoom::DeliverMessageUnsafe(const message_ &message)
{
  this->DeliverMessage(message.participant, message.content);
}

void ThreadSafeChatRoom::JoinUnsafe(
  const ChatRoomParticipantPtr &participant)
{
  this->Join(participant);
}

void ThreadSafeChatRoom::LeaveUnsafe(
  const ChatRoomParticipantPtr &participant)
{
  this->Leave(participant);
}

