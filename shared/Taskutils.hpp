#pragma once
#include <future>
#include "beatsaber-hook/shared/threading.hpp"

#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Threading/Tasks/TaskStatus.hpp"
#include "System/Exception.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include <chrono>
#include <cstddef>
#include <optional>
#include <thread>

#include <coroutine>

namespace TaskHelper {
// code heavily inspired from RedBrumbler

template <typename T> using Task = System::Threading::Tasks::Task_1<T>;

using System::Threading::CancellationToken;
using System::Threading::Tasks::TaskStatus;

namespace detail {

// Managed tasks record worker failures instead of letting an exception escape
// the native thread entry point.
template<class T, class F>
void complete_task(Task<T>* task, F&& work) {
  try {
    std::forward<F>(work)();
  } catch (const std::exception& error) {
    task->TrySetException(System::Exception::New_ctor(error.what()));
  } catch (...) {
    task->TrySetException(System::Exception::New_ctor("Unknown native exception"));
  }
}

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
    std::optional<Task<T> *> il2cpp_task;
    std::optional<T> result;
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
      il2cpp_task.reset();
      async_future = {};
      return std::suspend_always{};
    };

    template <typename U> std::suspend_never yield_value(U &&value) = delete;

    struct awaiter {
      promise_type& promise;
      bool await_ready() const { return promise.await_ready(); }
      void await_suspend(std::coroutine_handle<>) const noexcept {}
      T await_resume() {
        T value = promise.il2cpp_task ? promise.il2cpp_task.value()->Result : promise.async_future.get();
        promise.result = value;
        promise.il2cpp_task.reset();
        promise.async_future = {};
        return value;
      }
    };

    // Keep the pending operation in the promise while the driver polls it.
    auto await_transform(std::future<T> future) {
      async_future = std::move(future);
      il2cpp_task = std::nullopt;
      return awaiter{*this};
    }

    // TODO: Somehow make this work. This is desirable
    auto await_transform(Generator<T> generator) = delete;

    // handle il2cpp task
    auto await_transform(Task<T> *task) {
      il2cpp_task.emplace(task);
      async_future = {};

      return awaiter{*this};
    }

    // return true if the awaited future
    // is done
    bool await_ready() const {
      if (il2cpp_task.has_value()) {
        return il2cpp_task.value()->IsCompleted;
      }

      // A deferred future runs in await_resume; a plain yield has no operation.
      return !async_future.valid() ||
             async_future.wait_for(std::chrono::seconds(0)) != std::future_status::timeout;
    }
    void return_void() {}

    void rethrow_if_exception() {
      if (m_exception) {
        std::rethrow_exception(m_exception);
      }
    }
  };

  bool next() {
    handle.resume();
    return !handle.done();
  }

  std::optional<T> value_if_done() const {
    return handle.promise().result;
  }

  handle_type handle;
};
} // namespace detail

template <typename Ret, typename T>
static Task<Ret> *task_awaitable(T &&func, CancellationToken &&cancelToken) {
  auto task = Task<Ret>::New_ctor();

  // A detached worker lets the caller receive the managed task immediately.
  il2cpp_thread(
      [task, func = std::forward<T>(func), cancelToken]() mutable {
        detail::complete_task(task, [&] {
          detail::Generator<Ret> generator(func());
          while (!cancelToken.IsCancellationRequested && generator.next()) {
            while (!cancelToken.IsCancellationRequested && !generator.handle.promise().await_ready()) {
              std::this_thread::yield();
            }
          }
          if (!cancelToken.IsCancellationRequested) {
            generator.handle.promise().rethrow_if_exception();
            task->TrySetResult(generator.value_if_done().value());
          } else {
            task->TrySetCanceled(cancelToken);
          }
        });
      }).detach();

  return task;
}

template <typename Ret, typename T>
  requires(std::is_invocable_r_v<Ret, T>)
static void task_func(Task<Ret> *task, T &&func) {
  detail::complete_task(task, [&] {
    task->TrySetResult(std::invoke(std::forward<T>(func)));
  });
}

template <typename Ret, typename T>
  requires(std::is_invocable_r_v<Ret, T, CancellationToken>)
static void task_cancel_func(Task<Ret> *task, T &&func,
                             CancellationToken &&cancelToken) {
  using namespace std::chrono_literals;
  detail::complete_task(task, [&] {
    if (cancelToken.IsCancellationRequested) {
      task->TrySetCanceled(cancelToken);
      return;
    }
    auto fut = il2cpp_async(
        std::launch::async, std::forward<T>(func), CancellationToken(cancelToken));
    while (fut.wait_for(500ns) != std::future_status::ready &&
           !cancelToken.IsCancellationRequested)
      std::this_thread::yield();

    if (!cancelToken.IsCancellationRequested) {
      task->TrySetResult(fut.get());
    } else {
      task->TrySetCanceled(cancelToken);
    }
  });
}

template <typename Ret, typename T>
  requires(!std::is_same_v<Ret, void> && std::is_invocable_r_v<Ret, T>)
static Task<Ret> *StartTask(T &&func) {
  auto t = Task<Ret>::New_ctor();
  il2cpp_thread([t, func = std::forward<T>(func)]() mutable {
    task_func(t, std::move(func));
  }).detach();
  return t;
}

template <typename Ret, typename T>
  requires(!std::is_same_v<Ret, void> &&
           std::is_invocable_r_v<Ret, T, CancellationToken>)
static Task<Ret> *StartTask(T &&func, CancellationToken &&cancelToken) {
  auto t = Task<Ret>::New_ctor();
  il2cpp_thread([t, func = std::forward<T>(func), cancelToken]() mutable {
    task_cancel_func(t, std::move(func), std::move(cancelToken));
  }).detach();
  return t;
}
} // namespace TaskHelper
