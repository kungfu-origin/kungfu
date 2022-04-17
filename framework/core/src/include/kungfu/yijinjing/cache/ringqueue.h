
#pragma once

#include <atomic>
#include <cstddef>
#include <kungfu/common.h>
#include <mutex>
//#include <bitset>
// #include <iostream>

// using namespace std;
#if 0
namespace kungfu::yijinjing::cache {
template <typename T> class ringqueue {
public:
  explicit ringqueue(size_t capacity) : capacityMask_(capacity - 1), tail_(0), head_(0) {
    for (size_t i = 1; i <= sizeof(void *) * 4; i <<= 1) {
      capacityMask_ |= capacityMask_ >> i;
    }
    capacity_ = capacityMask_ + 1;
    queue_ = (T *)new char[sizeof(T) * capacity_];
    pop_value_ = (T *)new char[sizeof(T)];
  }

  ringqueue(ringqueue &&that)
      : capacity_(that.capacity_), capacityMask_(that.capacityMask_), head_(that.head_), tail_(that.tail_),
        queue_(that.queue_), pop_value_(that.pop_value_) {
    that.queue_ = nullptr;
    that.pop_value_ = nullptr;
  }

  ringqueue(const ringqueue &that)
      : capacity_(that.capacity_), capacityMask_(that.capacityMask_), head_(that.head_), tail_(that.tail_),
        queue_((T *)new char[sizeof(T) * that.capacity_]) {
    for (int i = 0; i < that.size(); i++) {
      T *node = queue_ + i;
      new (node) T(*(that.queue_ + i));
    }
    new (pop_value_) T(*(that.pop_value_));
  }

  ~ringqueue() {
    for (size_t i = head_; i != tail_; ++i)
      (&queue_[i & capacityMask_])->~T();
    (pop_value_)->~T();
    delete[](char *) queue_;
    delete[](char *) pop_value_;
  }

  size_t capacity() const { return capacity_; }

  size_t size() const { return tail_ - head_; }

  bool push(const T &p_data) {
    T *node;
    if (mtx_.try_lock()) {
      node = &queue_[tail_ & capacityMask_];
      if (tail_ - head_ >= capacity_) {
        head_ = tail_ + 1;
      }
      new (node) T(p_data);
      tail_ = tail_ + 1;
      mtx_.unlock();
      return true;
    } else {
      return false;
    }
  }

  bool pop(T *result) {
    if (head_ >= tail_) {
      return false;
    }
    T *node;
    result = nullptr;
    if (mtx_.try_lock()) {
      if (head_ >= tail_) {
        mtx_.unlock();
        return false;
      }
      node = &queue_[head_ & capacityMask_];
      head_ = head_ + 1;
      *pop_value_ = *node;
      result = pop_value_;
      node->~T();
      mtx_.unlock();
      return true;
    } else {
      return false;
    }
  }

private:
  size_t capacityMask_;
  T *queue_;
  T *pop_value_;
  size_t capacity_;
  volatile size_t tail_;
  volatile size_t head_;
  std::mutex mtx_;
};
} // namespace kungfu::yijinjing::cache
#endif
namespace kungfu::yijinjing::cache {
template <typename T> class ringqueue {
public:
  explicit ringqueue(size_t capacity) {
    capacityMask_ = capacity - 1;
    for (size_t i = 1; i <= sizeof(void *) * 4; i <<= 1) {
      capacityMask_ |= capacityMask_ >> i                                          ;
    }
    capacity_ = capacityMask_ + 1;
    queue_ = (T *)new char[sizeof(T) * capacity_];
    pop_value_ = (T *)new char[sizeof(T)];
    tail_.store(0, std::memory_order_relaxed);
    head_.store(0, std::memory_order_relaxed);
  }

  ringqueue(ringqueue &&that)
      : capacity_(that.capacity_), capacityMask_(that.capacityMask_), head_(that.head_.load(std::memory_order_relaxed)),
        tail_(that.tail_.load(std::memory_order_relaxed)), queue_(that.queue_), pop_value_(that.pop_value_) {
    that.queue_ = nullptr;
    that.pop_value_ = nullptr;
  }

  ringqueue(const ringqueue &that)
      : capacity_(that.capacity_), capacityMask_(that.capacityMask_), head_(that.head_.load(std::memory_order_relaxed)),
        tail_(that.tail_.load(std::memory_order_relaxed)), queue_((T *)new char[sizeof(T) * that.capacity_]) {
    for (int i = 0; i < that.size(); i++) {
      T *node = queue_ + i;
      new (node) T(*(that.queue_ + i));
    }
    new (pop_value_) T(*(that.pop_value_));
  }

  ~ringqueue() {
    for (size_t i = head_; i != tail_; ++i)
      (&queue_[i & capacityMask_])->~T();
    (pop_value_)->~T();
    delete[](char *) queue_;
    delete[](char *) pop_value_;
  }
  size_t capacity() const { return capacity_; }
  size_t size() const {
    size_t head = head_.load(std::memory_order_acquire);
    return tail_.load(std::memory_order_relaxed) - head;
  }

  bool push(const T &p_data) {
    T *node;
    size_t tail = tail_.load(std::memory_order_relaxed);
    size_t head = head_.load(std::memory_order_relaxed);
    node = &queue_[tail & capacityMask_];
    new (node) T(p_data);
    if (tail - head < capacity_ - 1) {
      tail_++;
    }
    return true;
  }

  bool pop(T *result) {
    T *node;
    result = nullptr;
    size_t head = head_.load(std::memory_order_relaxed);
    size_t tail = tail_.load(std::memory_order_relaxed);
    if (head >= tail) {
      // std::cout << "pop empty queue..........................." << std::endl;
      return false;
    }
    node = &queue_[head & capacityMask_];
    *pop_value_ = *node;
    result = pop_value_;
    node->~T();
    head_++;
    // std::cout << "pop " << result << " head<<" << head_ << " tail<<" << tail_ << std::endl;
    return true;
  }

private:
  size_t capacityMask_;
  T *queue_;
  T *pop_value_;
  size_t capacity_t ;
  std::atomic<size_t> tail_;
  std::atomic<size_t> head_;
};
} // namespace kungfu::yijinjing::cache