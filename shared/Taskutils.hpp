#pragma once

#include "System/Threading/Tasks/Task_1.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include <chrono>
#include <cstddef>
#include <thread>

#include <coroutine>

namespace TaskHelper {
// code heavily inspired from RedBrumbler

template <typename T> using Task = System::Threading::Tasks::Task_1<T>;

using System::Threading::CancellationToken;

namespace detail {

template <typename T> struct Generator {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  Generator(handle_type h) : handle(h) {}
  Generator(Generator &&generator) noexcept : handle(generator.handle) {
    generator.handle = nullptr;
  }
  ~Generator() {
    if (handle)
      handle.destroy();
  }

  struct promise_type {
    // store il2cpp_task separately to poll
    // instead of making a new thread only to call future->set_value()
    // I would prefer making my own std::future or something else entirely

    std::future<T> async_future;

    // TODO: Allow any task to work? currently only allows Task<T>
    std::optional<Task<T>*> il2cpp_task;
    std::exception_ptr m_exception;

    auto get_return_object() {
      return Generator{handle_type::from_promise(*this)};
    }
    auto initial_suspend() { return std::suspend_always{}; }
    auto final_suspend() noexcept { return std::suspend_always{}; }
    void unhandled_exception() noexcept {
      m_exception = std::current_exception();
    }

    // Yield control and return if cancellation
    std::suspend_always yield_value(std::nullptr_t) {
      return std::suspend_always{};
    };

    template <typename U>
    std::suspend_never yield_value(U &&value) = delete;

    // handle await
    auto await_transform(std::future<T> future) {
      async_future = std::move(future);
      il2cpp_task = std::nullopt;
      return std::suspend_always{};
    }

    // TODO: Somehow make this work. This is desirable 
    auto await_transform(Generator<T> generator) = delete;

    // handle il2cpp task
    auto await_transform(Task<T> *task) {
      il2cpp_task.emplace(task);
      async_future = {};

      return std::suspend_always{};
    }

    // return true if the awaited future
    // is done
    auto await_ready() const {
     if (il2cpp_task.has_value()) {
       return il2cpp_task.value()->IsCompleted;
     }

     return async_future.valid() &&
              async_future.wait_for(std::chrono::seconds(0)) ==
                  std::future_status::ready;
    }
    auto await_resume() { return async_future.get(); }
    void return_void() {}

    void rethrow_if_exception() {
      if (il2cpp_task.has_value()) {
        // will raise exception
        // TODO: Grab exception
        il2cpp_task.value()->Wait();
      }
      
      if (m_exception) {
        std::rethrow_exception(m_exception);
      }
    }
  };

  bool next() {
    handle.resume();
    return !handle.done();
  }

  // not sure if this even works
  T value() const { return handle.promise().current_value; }

  handle_type handle;
};
} // namespace detail

template <typename Ret, typename T>
static Task<Ret>* task_awaitable(T &&func, CancellationToken &&cancelToken) {
  using namespace std::chrono_literals;
  auto task = Task<Ret>::New_ctor();

  // start func as an std::future
  auto fut = il2cpp_utils::il2cpp_async(
      [task](T &&func, CancellationToken &&cancelToken) {
        // make a generator
        detail::Generator<Ret> generator(std::forward<T>(func));

        // while there is work to do, continue
        while (!cancelToken.IsCancellationRequested && generator.next()) {
          // yield control back to the async runtime
          std::this_thread::yield();
        }

        // TODO: Move this to the task exception

        generator.handle.rethrow_if_exception();
        // if cancellation wasn't requested, set result, else set canceled
        if (!cancelToken.IsCancellationRequested) {
          task->TrySetResult(generator.value());
        } else {
          task->TrySetCanceled(cancelToken);
        }
      },
      std::forward<T>(func), std::forward<CancellationToken>(cancelToken));

  return task;
}

template <typename Ret, typename T>
  requires(std::is_invocable_r_v<Ret, T>)
static void task_func(Task<Ret> *task, T &&func) {
  task->TrySetResult(std::invoke(std::forward<T>(func)));
}

template <typename Ret, typename T>
  requires(std::is_invocable_r_v<Ret, T, CancellationToken>)
static void task_cancel_func(Task<Ret> *task, T &&func,
                             CancellationToken &&cancelToken) {
  using namespace std::chrono_literals;
  // start func as an std::future
  auto fut = il2cpp_utils::il2cpp_async(
      std::forward<T>(func), std::forward<CancellationToken>(cancelToken));
  // await the future
  while (fut.wait_for(500ns) != std::future_status::ready &&
         !cancelToken.IsCancellationRequested)
    std::this_thread::yield();

  // if cancellation wasn't requested, set result, else set canceled
  if (!cancelToken.IsCancellationRequested) {
    task->TrySetResult(fut.get());
  } else {
    task->TrySetCanceled(cancelToken);
  }
}

template <typename Ret, typename T>
  requires(!std::is_same_v<Ret, void> && std::is_invocable_r_v<Ret, T>)
static Task<Ret> *StartTask(T &&func) {
  auto t = Task<Ret>::New_ctor();
  il2cpp_utils::il2cpp_aware_thread(&task_func<Ret, T>, t,
                                    std::forward<T>(func))
      .detach();
  return t;
}

template <typename Ret, typename T>
  requires(!std::is_same_v<Ret, void> &&
           std::is_invocable_r_v<Ret, T, CancellationToken>)
static Task<Ret> *StartTask(T &&func, CancellationToken &&cancelToken) {
  auto t = Task<Ret>::New_ctor();
  il2cpp_utils::il2cpp_aware_thread(
      &task_cancel_func<Ret, T>, t, std::forward<T>(func),
      std::forward<CancellationToken>(cancelToken))
      .detach();
  return t;
}
} // namespace TaskHelper