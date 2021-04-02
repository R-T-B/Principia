
#pragma once

#include "physics/checkpointer.hpp"

#include "astronomy/epoch.hpp"

namespace principia {
namespace physics {
namespace internal_checkpointer {

using astronomy::InfiniteFuture;

template<typename Message>
Checkpointer<Message>::Checkpointer(Reader reader, Writer writer)
    : reader_(std::move(reader)),
      writer_(std::move(writer)) {}

template<typename Message>
Instant Checkpointer<Message>::oldest_checkpoint() const {
  absl::ReaderMutexLock l(&lock_);
  if (checkpoints_.empty()) {
    return InfiniteFuture;
  }
  return checkpoints_.cbegin()->first;
}

template<typename Message>
bool Checkpointer<Message>::ReadFromOldestCheckpoint() const {
  absl::ReaderMutexLock l(&lock_);
  if (!checkpoints_.empty()) {
    reader_(checkpoints_.cbegin()->second);
    return true;
  }
  return false;
}

template<typename Message>
void Checkpointer<Message>::WriteToCheckpoint(Instant const& t) {
  absl::MutexLock l(&lock_);
  WriteToCheckpointLocked(t);
}

template<typename Message>
bool Checkpointer<Message>::WriteToCheckpointIfNeeded(
    Instant const& t,
    Time const& max_time_between_checkpoints) {
  absl::MutexLock l(&lock_);
  if (checkpoints_.empty() ||
      max_time_between_checkpoints < t - checkpoints_.crbegin()->first) {
    WriteToCheckpointLocked(t);
    return true;
  }
  return false;
}

template<typename Message>
void Checkpointer<Message>::WriteToMessage(
    not_null<google::protobuf::RepeatedPtrField<typename Message::Checkpoint>*>
        message) const {
  absl::ReaderMutexLock l(&lock_);
  for (const auto [time, checkpoint] : checkpoints_) {
    typename Message::Checkpoint* const message_checkpoint = message->Add();
    *message_checkpoint = checkpoint;
    time.WriteToMessage(message_checkpoint->mutable_time());
  }
}

template<typename Message>
not_null<std::unique_ptr<Checkpointer<Message>>>
Checkpointer<Message>::ReadFromMessage(
    Reader reader,
    Writer writer,
    google::protobuf::RepeatedPtrField<typename Message::Checkpoint> const&
        message) {
  auto checkpointer =
      std::make_unique<Checkpointer>(std::move(reader), std::move(writer));
  for (const auto& checkpoint : message) {
    Instant const time = Instant::ReadFromMessage(checkpoint.time());
    checkpointer->checkpoints_.emplace(time, checkpoint);
  }
  return std::move(checkpointer);
}

template<typename Message>
void Checkpointer<Message>::WriteToCheckpointLocked(Instant const& t) {
  lock_.AssertHeld();
  auto const it = checkpoints_.emplace_hint(
      checkpoints_.end(), t, typename Message::Checkpoint());
  writer_(&it->second);
}

}  // namespace internal_checkpointer
}  // namespace physics
}  // namespace principia
