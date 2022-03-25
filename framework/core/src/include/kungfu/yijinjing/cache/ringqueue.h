
#pragma once

#include <atomic>
#include <cstddef>
#include <kungfu/common.h>
//#include <bitset>
// #include <iostream>

// using namespace std;
namespace kungfu::yijinjing::cache {
template <typename T> class ringqueue
{
public:
  explicit ringqueue(size_t capacity) {
	  SPDLOG_INFO("ringqueue 1");
    _capacityMask = capacity - 1;
    // std::cout << "_capacityMask=" << _capacityMask << " " << std::bitset<sizeof(_capacityMask) * 8>(_capacityMask) <<
    // std::endl;
    for (size_t i = 1; i <= sizeof(void *) * 4; i <<= 1) {
      _capacityMask |= _capacityMask >> i;
      // std::cout << "i=" << i << " _capacityMask=" << _capacityMask << " " << bitset<sizeof(_capacityMask) *
      // 8>(_capacityMask) << std::endl;
    }
    _capacity = _capacityMask + 1;
    // std::cout << "_capacity=" << _capacity << std::endl;
	  SPDLOG_INFO("ringqueue 2");
    _queue = (T *)new char[sizeof(T) * _capacity];
	  SPDLOG_INFO("ringqueue 3");
	pop_value_ = (T *)new char[sizeof(T)];
	  SPDLOG_INFO("ringqueue 4");
    _tail.store(0, std::memory_order_relaxed);
    _head.store(0, std::memory_order_relaxed);
	  SPDLOG_INFO("ringqueue 5");
	}

	ringqueue(ringqueue &&that)
		: _capacity(that._capacity), _capacityMask(that._capacityMask),
			_head(that._head.load(std::memory_order_relaxed)), _tail(that._tail.load(std::memory_order_relaxed)),
			_queue(that._queue), pop_value_(that.pop_value_) {
		that._queue = nullptr;
		that.pop_value_ = nullptr;
	}

        ringqueue(const ringqueue &that)
            : _capacity(that._capacity), _capacityMask(that._capacityMask),
              _head(that._head.load(std::memory_order_relaxed)), _tail(that._tail.load(std::memory_order_relaxed)),
              _queue((T *)new char[sizeof(T) * that._capacity]) {
          for (int i = 0; i < that.size(); i++) {
            T *node = _queue + i;
            new (node) T(*(that._queue + i));
          }
          new (pop_value_) T(*(that.pop_value_));
        }

        ~ringqueue() {
          for (size_t i = _head; i != _tail; ++i)
            (&_queue[i & _capacityMask])->~T();
          (pop_value_)->~T();
          delete[](char *) _queue;
          delete[](char *) pop_value_;
        }

        size_t capacity() const { return _capacity; }

	size_t size() const
	{
		size_t head = _head.load(std::memory_order_acquire);
		return _tail.load(std::memory_order_relaxed) - head;
	}
	bool push(const T& p_data)
	{
		T* node;
		for (;;)
		{
			size_t tail = _tail.load(std::memory_order_relaxed);
			size_t head = _head.load(std::memory_order_relaxed);
			node = &_queue[tail & _capacityMask];
			if (tail - head >= _capacity) {
				if (!_head.compare_exchange_weak(head, tail + 1 - _capacity, std::memory_order_relaxed)) {
					// std::cout << "push " << p_data << " head from " << head << " to " << tail + 1 - _capacity  << std::endl;
					continue;
				}
			}
			new (node)T(p_data);
			_tail++;
			break;
		}
		// std::cout << "push " << p_data << " _head>>" << _head << " _tail>>" << _tail << std::endl;
		return true;
	}

	bool pop(T* result)
	{
		T* node;
		result = nullptr;
		int i = 0;
		for (i = 0; i < 128; i++)
		{
			size_t head = _head.load(std::memory_order_relaxed);
			size_t tail = _tail.load(std::memory_order_relaxed);
			if (head >= tail) {
				// std::cout << "pop empty queue..........................." << std::endl;
				return false;
			}
			node = &_queue[head & _capacityMask];
			if (_head.compare_exchange_weak(head, head + 1, std::memory_order_relaxed)) {
				*pop_value_ = *node;
				result = pop_value_;
				node->~T();
			
				break;
			}				
		}
		if (i == 128) {
			// std::cout << "pop 128 fail..........................." << std::endl;
			return  false;
		}
		// std::cout << "pop " << result << " head<<" << _head << " tail<<" << _tail << std::endl;
		return true;
	}
private:
	size_t _capacityMask;
	T* _queue;
	T* pop_value_;
	size_t _capacity;
	std::atomic<size_t> _tail;
	std::atomic<size_t> _head;
};
}