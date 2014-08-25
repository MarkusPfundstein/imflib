#ifndef BOUND_QUEUE_HPP_INCLUDED
#define BOUND_QUEUE_HPP_INCLUDED

#include <mutex>
#include <condition_variable>
#include <boost/circular_buffer.hpp>
struct operation_aborted {};

template <class T, std::size_t N>
class bound_queue {
public:
  typedef T value_type;
  bound_queue() : q_(N), aborted_(false) {}
  void push(value_type data)
  {
    std::unique_lock<std::mutex> lk(mtx_);
    cv_pop_.wait(lk, [=]{ return !q_.full() || aborted_; });
    if (aborted_) throw operation_aborted();
    q_.push_back(data);
    cv_push_.notify_one();
  }
  value_type pop()
  {
    std::unique_lock<std::mutex> lk(mtx_);
    cv_push_.wait(lk, [=]{ return !q_.empty() || aborted_; });
    if (aborted_) throw operation_aborted();
    value_type result = q_.front();
    q_.pop_front();
    cv_pop_.notify_one();
    return result;
  }
  void abort()
  {
    std::lock_guard<std::mutex> lk(mtx_);
    aborted_ = true;
    cv_pop_.notify_all();
    cv_push_.notify_all();
  }
  int size() {
    std::lock_guard<std::mutex> lk(mtx_);
    return q_.size();
  }

private:
  boost::circular_buffer<value_type> q_;
  bool aborted_;
  std::mutex mtx_;
  std::condition_variable cv_push_;
  std::condition_variable cv_pop_;
};


#endif // BOUND_QUEUE_HPP_INCLUDED
