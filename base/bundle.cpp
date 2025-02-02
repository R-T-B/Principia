#include "base/bundle.hpp"

#include <functional>
#include <list>

#include "absl/status/status.h"
#include "glog/logging.h"

namespace principia {
namespace base {
namespace _bundle {
namespace internal {

void Bundle::Add(Task task) {
  absl::MutexLock l(&lock_);
  CHECK(!joining_);
  ++number_of_active_workers_;
  workers_.emplace_back(&Bundle::Toil, this, std::move(task));
}

absl::Status Bundle::Join() {
  joining_ = true;
  if (number_of_active_workers_ > 0) {
    all_done_.WaitForNotification();
  }
  JoinAll();
  absl::ReaderMutexLock status_lock(&status_lock_);
  return status_;
}

absl::Status Bundle::JoinWithin(std::chrono::steady_clock::duration Δt) {
  joining_ = true;
  if (number_of_active_workers_ > 0 &&
      !all_done_.WaitForNotificationWithTimeout(absl::FromChrono(Δt))) {
    absl::MutexLock l(&status_lock_);
    status_ = absl::DeadlineExceededError("Bundle deadline exceeded");
  }
  JoinAll();
  absl::ReaderMutexLock status_lock(&status_lock_);
  return status_;
}

absl::Status Bundle::JoinBefore(std::chrono::system_clock::time_point t) {
  joining_ = true;
  if (number_of_active_workers_ > 0 &&
      !all_done_.WaitForNotificationWithDeadline(absl::FromChrono(t))) {
    absl::MutexLock l(&status_lock_);
    status_ = absl::DeadlineExceededError("bundle deadline exceeded");
  }
  JoinAll();
  absl::ReaderMutexLock status_lock(&status_lock_);
  return status_;
}

void Bundle::Toil(Task const& task) {
  absl::Status const status = task();

  // Avoid locking if the task succeeded: it cannot affect the overall status.
  if (!status.ok()) {
    absl::MutexLock l(&status_lock_);
    status_.Update(status);
  }

  // No locking, so as to avoid contention during joining.  Note that if
  // |joining_| is true we know that |number_of_active_workers_| cannot
  // increase.  Reading |number_of_active_workers_| independently from the
  // decrement would be incorrect as we must ensure that exactly one thread sees
  // that counter dropping to zero.
  int const number_of_active_workers = --number_of_active_workers_;
  if (joining_ && number_of_active_workers == 0) {
    all_done_.Notify();
  }
}

void Bundle::JoinAll() {
  absl::ReaderMutexLock l(&lock_);
  for (auto& worker : workers_) {
    worker.join();
  }
}

}  // namespace internal
}  // namespace _bundle
}  // namespace base
}  // namespace principia
