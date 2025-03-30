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
#include <atomic>

template <typename T>
class SafeQueue {
 public:
    inline SafeQueue() : waiters_(0) {}
    inline T pop() {
        waiters_.fetch_add(1);
        popBase();
        waiters_.fetch_sub(1);
    }

    inline T pop(const std::condition_variable &signal) {
        waiters_.fetch_add(1);
        signal.notify_one();
        popBase();
        waiters_.fetch_sub(1);
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
    inline int idleAndEmpty(int maxThreads) {
        std::unique_lock<std::mutex> scopeLock(queue_lock_);
        return maxThreads == queue_.size();
    }

 private:
    inline T popBase() {
        std::unique_lock<std::mutex> scopeLock(queue_lock_);
        item_available_.wait(scopeLock, [this] { return !queue_.empty(); });
        assert(!queue_.empty());
        auto item = queue_.front();
        queue_.pop();
        return item;
    }
    std::mutex queue_lock_;
    std::queue<T> queue_;
    std::condition_variable item_available_;
    
    std::atomic<int> waiters_;
};
