
#pragma once

#include <atomic>
#include <cstddef>
#include <kungfu/common.h>
#include <mutex>
//#include <bitset>
// #include <iostream>

// using namespace std;
namespace kungfu::yijinjing::cache {
template <typename T> class ringqueue {
public:
  explicit ringqueue(size_t capacity) : capacityMask_(capacity - 1), tail_(0), head_(0) {
    // std::cout << "capacityMask_=" << capacityMask_ << " " << std::bitset<sizeof(capacityMask_) * 8>(capacityMask_) <<
    // std::endl;
    for (size_t i = 1; i <= sizeof(void *) * 4; i <<= 1) {
      capacityMask_ |= capacityMask_ >> i;
      // std::cout << "i=" << i << " capacityMask_=" << capacityMask_ << " " << bitset<sizeof(capacityMask_) *
      // 8>(capacityMask_) << std::endl;
    }
    capacity_ = capacityMask_ + 1;
    // std::cout << "capacity_=" << capacity_ << std::endl;
    queue_ = (T *)new char[sizeof(T) * capacity_];
    pop_value_ = (T *)new char[sizeof(T)];
    // SPDLOG_INFO("ringqueue 1 capacity_ {} capacityMask_ {}", capacity_, capacityMask_);
  }

  ringqueue(ringqueue &&that)
      : capacity_(that.capacity_), capacityMask_(that.capacityMask_), head_(that.head_), tail_(that.tail_),
        queue_(that.queue_), pop_value_(that.pop_value_) {
    that.queue_ = nullptr;
    that.pop_value_ = nullptr;
    // SPDLOG_INFO("ringqueue 2 capacity_ {} capacityMask_ {}", capacity_, capacityMask_);
  }

  ringqueue(const ringqueue &that)
      : capacity_(that.capacity_), capacityMask_(that.capacityMask_), head_(that.head_), tail_(that.tail_),
        queue_((T *)new char[sizeof(T) * that.capacity_]) {
    for (int i = 0; i < that.size(); i++) {
      T *node = queue_ + i;
      new (node) T(*(that.queue_ + i));
    }
    new (pop_value_) T(*(that.pop_value_));
    // SPDLOG_INFO("ringqueue 3 capacity_ {} capacityMask_ {}", capacity_, capacityMask_);
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
      // SPDLOG_INFO("push head_ {} tail_ {}, p_data {} capacity_ {} capacityMask_ {}", head_, tail_, p_data.update_time, capacity_, capacityMask_);
      new (node) T(p_data);
      // SPDLOG_INFO("push head_ {} tail_ {}", head_, tail_);
      tail_++;
      mtx_.unlock();
      return true;
    } else {
      return false;
    }
    // std::cout << "push " << p_data << " head_>>" << head_ << " tail_>>" << tail_ << std::endl;
  }

  bool pop(T *result) {
    if (head_ >= tail_) {
      // std::cout << "pop empty queue..........................." << std::endl;
      return false;
    }
    T *node;
      // SPDLOG_INFO("pop head_ 1 {} tail_ {}", head_, tail_);
    result = nullptr;
    if (mtx_.try_lock()) {
      // SPDLOG_INFO("pop head_ 2 {} tail_ {}", head_, tail_);
      if (head_ >= tail_) {
        // std::cout << "pop empty queue..........................." << std::endl;
        mtx_.unlock();
        return false;
      }
      node = &queue_[head_ & capacityMask_];
      head_++;
      *pop_value_ = *node;
      result = pop_value_;
      node->~T();
      mtx_.unlock();
      return true;
    } else {
      return false;
    }
    // std::cout << "pop " << result << " head<<" << head_ << " tail<<" << tail_ << std::endl;
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