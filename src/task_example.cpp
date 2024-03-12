// Include headers so CLion doesn't cry

#include "Taskutils.hpp"

TaskHelper::detail::Generator<void> bar() {
  co_return;
}

TaskHelper::detail::Generator<void> awaitableTask() {
  TaskHelper::Task<void>* randomTask = nullptr;
  co_await randomTask;
  // task was done, no-op
  co_await randomTask;

  // do work
  for (int i = 0; i < 10; i++) {
    // yield task
    // if cancellation was requested, this will stop
    co_yield nullptr;
  }
  
  // chain
//   co_await bar();


  
  co_return;
}

int main() {
  
}
