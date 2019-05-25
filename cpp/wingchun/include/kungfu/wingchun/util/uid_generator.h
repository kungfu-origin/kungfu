//
// Created by qlu on 2019/1/17.
//

#ifndef WC_2_UID_GENERATOR_H
#define WC_2_UID_GENERATOR_H

#include <string>
#include "assert.h"
#include <stdexcept>

namespace kungfu
{
    struct UidWorker
    {
        int id;
        std::string name;
    };

    class UidWorkerStorage
    {
    public:
        static UidWorkerStorage* get_instance(const std::string& file_name);
        void add_uid_worker(const std::string& name);
        int get_uid_worker_id(const std::string& name);
    private:
        static UidWorkerStorage* instance_;
        UidWorkerStorage() = delete;
        UidWorkerStorage& operator=(UidWorkerStorage const&) = delete;
        UidWorkerStorage(const std::string& file_name);
        std::string file_name_;
        void create_table_if_not_exist();
    };

    class BitAllocator
    {
    public:
        BitAllocator(int timestamp_bits = 28, int worker_id_bits = 13, int sequence_bits = 22)
        {
            assert((timestamp_bits + worker_id_bits + sequence_bits + kSignBits) == kTotalBits);

            timestamp_bits_ = timestamp_bits;
            worker_id_bits_ = worker_id_bits;
            sequence_bits_ = sequence_bits;

            max_delta_second_ = (1 << timestamp_bits) -1;
            max_worker_id_ = (1 << worker_id_bits) -1;
            max_sequence_ = (1 << sequence_bits) -1;

            timestamp_shift_ = worker_id_bits + sequence_bits;
            worker_id_shift_ = sequence_bits;
            sign_shift_ = kTotalBits -1;
        }

        int64_t allocate(int64_t delta_seconds, int64_t worker_id, int64_t sequence) const
        {
            return (delta_seconds << timestamp_shift_ | worker_id << worker_id_bits_) | sequence;
        }

        int64_t get_max_sequence() const { return this->max_sequence_; }
        int64_t get_max_delta_seconds() const { return  this->max_delta_second_; }
        int64_t get_max_worker_id() const { return this->max_worker_id_; }
    private:
        static const int kTotalBits = 1 << 6;
        static const  int kSignBits = 1;

        int timestamp_bits_;
        int worker_id_bits_;
        int sequence_bits_;

        int64_t max_delta_second_;
        int64_t max_worker_id_;
        int64_t max_sequence_;

        int timestamp_shift_;
        int worker_id_shift_;
        int sign_shift_;
    };

    class UidGenerator
    {
    public:
        UidGenerator(const int worker_id, const int64_t epoch_seconds): worker_id_(worker_id), last_seconds_(0), epoch_seconds_(epoch_seconds)
        {
            assert(worker_id <= bit_allocator_.get_max_worker_id());
        }

        UidGenerator(const UidGenerator&) = delete;
        UidGenerator& operator=(const UidGenerator&) = delete;

        inline int64_t next_id(int64_t current_seconds)
        {
            if (current_seconds < last_seconds_)
            {
                throw std::runtime_error("clock moved backwards, refusing seconds " + std::to_string(current_seconds));
            }
            if (current_seconds == last_seconds_)
            {
                sequence_ = (sequence_ + 1) & bit_allocator_.get_max_sequence();
                if (sequence_ == 0)
                {
                    throw std::runtime_error("sequence bits is exhausted, max sequence " + std::to_string(bit_allocator_.get_max_sequence()));
                }
            }
            else
            {
                sequence_ = 0;
            }

            last_seconds_ = current_seconds;
            if (current_seconds - epoch_seconds_ > bit_allocator_.get_max_delta_seconds())
            {
                throw std::runtime_error("timestamp bits is exhausted");
            }
            return bit_allocator_.allocate(current_seconds - epoch_seconds_, worker_id_, sequence_);
        }

    private:
        int64_t worker_id_;

        int64_t last_seconds_;
        int64_t sequence_;

        BitAllocator bit_allocator_;

        int64_t epoch_seconds_;
    };
}
#endif //WC_2_UID_GENERATOR_H
