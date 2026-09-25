#include <coroutine>
#include <exception>
namespace coroutine {
template <typename T> struct Task {
  struct promise_type {
    T value_;
    std::exception_ptr exception_;
    // 存储非 void 类型的值
    std::coroutine_handle<> continuation_; // 存储恢复点

    Task get_return_object() {
      return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
    }
    std::suspend_always initial_suspend() { return {}; } // 协程开始时暂停
    std::suspend_always final_suspend() noexcept {
      return {};
    } // 协程结束时暂停

    void return_value(T value) { value_ = value; }
    void unhandled_exception() { exception_ = std::current_exception(); }
  };

  std::coroutine_handle<promise_type> handle_;

  Task(std::coroutine_handle<promise_type> h) : handle_(h) {}
  ~Task() {
    if (handle_)
      handle_.destroy();
  } // 销毁协程状态

  T get() {
    handle_.resume();
    if (handle_.promise().exception_) {
      std::rethrow_exception(handle_.promise().exception_);
    }
    return handle_.promise().value_;
  }

  // 实现 co_await 支持
  bool await_ready() const { return false; }
  void await_suspend(std::coroutine_handle<> h) {
    handle_.promise().continuation_ = h;
    handle_.resume();
  }
  T await_resume() {
    if (handle_.promise().exception_) {
      std::rethrow_exception(handle_.promise().exception_);
    }
    return handle_.promise().value_;
  }
};
} // namespace coroutine
