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
    SafeQueue();
    T pop();
    void push(T item);
 private:
    std::mutex queue_lock_;
    std::queue<T> queue_;
};
