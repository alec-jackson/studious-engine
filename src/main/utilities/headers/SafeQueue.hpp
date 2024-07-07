/**
 * @file SafeQueue.hpp
 * @author Christian Galvez
 * @brief Small wrapper class for std::queues that implement
 * @version 0.1
 * @date 2024-04-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>

template <typename T>
class SafeQueue {
 public:
    inline SafeQueue() {}
    inline T pop() {
        std::unique_lock<std::mutex> scopeLock(queue_lock_);
        item_available_.wait(scopeLock, [this] { return !queue_.empty(); });
        assert(!queue_.empty());
        auto item = queue_.front();
        queue_.pop();
        return item;
    }
    inline void push(T item) {
        std::unique_lock<std::mutex> scopeLock(queue_lock_);
        queue_.push(item);
        item_available_.notify_one();
    }
    inline int size() {
        std::unique_lock<std::mutex> scopeLock(queue_lock_);
        return queue_.size();
    }

 private:
    std::mutex queue_lock_;
    std::queue<T> queue_;
    std::condition_variable item_available_;
};
