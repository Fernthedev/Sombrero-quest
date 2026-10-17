#pragma once
#include <future>
#include "beatsaber-hook/shared/threading.hpp"

#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Threading/Tasks/TaskStatus.hpp"
#include "System/Threading/Tasks/TaskCanceledException.hpp"
#include "System/Exception.hpp"
#include "beatsaber-hook/shared/members.hpp"
#include "beatsaber-hook/shared/arrayw.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include <chrono>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <thread>

#include <coroutine>

namespace TaskHelper {
// code heavily inspired from RedBrumbler

template <typename T> using Task = System::Threading::Tasks::Task_1<T>;

using System::Threading::CancellationToken;
using System::Threading::Tasks::TaskStatus;

namespace detail {

// Coroutine frames and thread closures are native allocations, so managed
// references inside copied values need a root even while no stack local uses
// them. A managed slot also traces references inside value types such as tokens.
template<class T> struct managed_value_root {
  safe_ptr<ArrayW<T>, false> slot;
  explicit managed_value_root(T value) : slot(ArrayW<T>(1)) { set(value); }
  void set(T const& value) { slot.ptr()[0] = value; }
};
template<class T> struct managed_task_cancellation {
  safe_ptr<Task<T>*, false> child;
};

// Managed tasks record worker failures instead of letting an exception escape
// the native thread entry point.
template<class T, class F>
void complete_task(Task<T>* task, F&& work) {
  try {
    std::forward<F>(work)();
  } catch (const managed_task_cancellation<T>& canceled) {
    safe_ptr<System::Threading::Tasks::TaskCanceledException*, false> error(
        System::Threading::Tasks::TaskCanceledException::New_ctor(canceled.child.ptr()));
    task->TrySetCanceled(error->get_CancellationToken());
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
    close();
  }
  void close() {
    if (handle) {
      // A launch::async future can own work that still uses coroutine locals.
      // Release/join it while the entire frame and its managed roots are alive,
      // before destroy() starts running local and promise destructors.
      handle.promise().async_future = {};
      handle.destroy();
    }
    handle = nullptr;
  }

  struct promise_type {
    // store il2cpp_task separately to poll
    // instead of making a new thread only to call future->set_value()
    // I would prefer making my own std::future or something else entirely

    std::future<T> async_future;

    // TODO: Allow any task to work? currently only allows Task<T>
    std::optional<Task<T> *> il2cpp_task;
    std::optional<safe_ptr<Task<T>*, false>> il2cpp_task_root;
    std::optional<T> result;
    std::optional<managed_value_root<T>> result_root;
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
      il2cpp_task_root.reset();
      async_future = {};
      return std::suspend_always{};
    };

    template <typename U> std::suspend_never yield_value(U &&value) = delete;

    struct awaiter {
      promise_type& promise;
      bool await_ready() const { return promise.await_ready(); }
      void await_suspend(std::coroutine_handle<>) const noexcept {}
      T await_resume() {
        if (promise.il2cpp_task && promise.il2cpp_task.value()->get_IsCanceled()) {
          throw managed_task_cancellation<T>{safe_ptr<Task<T>*, false>(promise.il2cpp_task.value())};
        }
        T value = promise.il2cpp_task ? promise.il2cpp_task.value()->Result : promise.async_future.get();
        if (promise.result_root) promise.result_root->set(value);
        else promise.result_root.emplace(value);
        promise.result = value;
        promise.il2cpp_task.reset();
        promise.il2cpp_task_root.reset();
        promise.async_future = {};
        return value;
      }
    };

    // Keep the pending operation in the promise while the driver polls it.
    auto await_transform(std::future<T> future) {
      // Calling get() without a shared state is not guaranteed to throw;
      // Android's libc++ dereferences it. Fault the managed task consistently.
      if (!future.valid()) throw std::future_error(std::future_errc::no_state);
      async_future = std::move(future);
      il2cpp_task = std::nullopt;
      il2cpp_task_root.reset();
      return awaiter{*this};
    }

    // TODO: Somehow make this work. This is desirable
    auto await_transform(Generator<T> generator) = delete;

    // handle il2cpp task
    auto await_transform(Task<T> *task) {
      if (!task) throw std::invalid_argument("Cannot await a null managed task");
      il2cpp_task_root.emplace(task);
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
  safe_ptr<Task<Ret>*, false> taskRoot(task);
  detail::managed_value_root<CancellationToken> tokenRoot(cancelToken);

  // A detached worker lets the caller receive the managed task immediately.
  il2cpp_thread(
      [taskRoot = std::move(taskRoot), tokenRoot = std::move(tokenRoot), func = std::forward<T>(func), cancelToken]() mutable {
        auto* task = taskRoot.ptr();
        detail::complete_task(task, [&] {
          detail::Generator<Ret> generator(func());
          while (!cancelToken.IsCancellationRequested && generator.next()) {
            while (!cancelToken.IsCancellationRequested && !generator.handle.promise().await_ready()) {
              std::this_thread::yield();
            }
          }
          if (!cancelToken.IsCancellationRequested) {
            generator.handle.promise().rethrow_if_exception();
            auto result = generator.value_if_done().value();
            // Parameter destructors run when the frame is destroyed, even
            // after co_return. Keep the result rooted through that cleanup,
            // then publish completion after all frame-owned work is finished.
            auto resultRoot = std::move(generator.handle.promise().result_root);
            generator.close();
            task->TrySetResult(result);
          } else {
            // Destroy the frame before signaling cancellation. In particular,
            // a future created with launch::async may join its callable here.
            generator.close();
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
  detail::complete_task(task, [&] {
    if (cancelToken.IsCancellationRequested) {
      task->TrySetCanceled(cancelToken);
      return;
    }
    // This is already an attached worker. Run the callable here so completion
    // never races ahead of a second worker still using its captured data.
    std::optional<Ret> result;
    std::exception_ptr failure;
    try {
      result.emplace(std::invoke(std::forward<T>(func), CancellationToken(cancelToken)));
    } catch (...) {
      failure = std::current_exception();
    }
    if (cancelToken.IsCancellationRequested) {
      task->TrySetCanceled(cancelToken);
    } else {
      if (failure) std::rethrow_exception(failure);
      task->TrySetResult(std::move(result).value());
    }
  });
}

template <typename Ret, typename T>
  requires(!std::is_same_v<Ret, void> && std::is_invocable_r_v<Ret, T>)
static Task<Ret> *StartTask(T &&func) {
  auto t = Task<Ret>::New_ctor();
  safe_ptr<Task<Ret>*, false> taskRoot(t);
  il2cpp_thread([taskRoot = std::move(taskRoot), func = std::forward<T>(func)]() mutable {
    task_func(taskRoot.ptr(), std::move(func));
  }).detach();
  return t;
}

template <typename Ret, typename T>
  requires(!std::is_same_v<Ret, void> &&
           std::is_invocable_r_v<Ret, T, CancellationToken>)
static Task<Ret> *StartTask(T &&func, CancellationToken &&cancelToken) {
  auto t = Task<Ret>::New_ctor();
  safe_ptr<Task<Ret>*, false> taskRoot(t);
  detail::managed_value_root<CancellationToken> tokenRoot(cancelToken);
  il2cpp_thread([taskRoot = std::move(taskRoot), tokenRoot = std::move(tokenRoot), func = std::forward<T>(func), cancelToken]() mutable {
    task_cancel_func(taskRoot.ptr(), std::move(func), std::move(cancelToken));
  }).detach();
  return t;
}
} // namespace TaskHelper
