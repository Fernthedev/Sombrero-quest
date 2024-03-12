// Include headers so CLion doesn't cry

#include "Taskutils.hpp"

TaskHelper::detail::Generator<int> bar() {
  co_return;
}

TaskHelper::detail::Generator<int> awaitableTask() {
  TaskHelper::Task<int>* randomTask = nullptr;
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
  TaskHelper::task_awaitable<int>(&awaitableTask, TaskHelper::CancellationToken());
}
