/**
 * @file ProcessMgr.cpp
 * @brief Simple object for managing thread pool task execution.
 * @date 03-11-2026
 * @author Christian Galvez
 * @copyright Studious Engine 2026
 */

#include <ProcessMgr.hpp>
#include <mutex>  //NOLINT
#include <cstdio>

ProcessMgr::ProcessMgr(uint numThreads) : numThreads_ { numThreads }, freeWorkers_ { numThreads } {
    for (uint i = 0; i < numThreads; ++i) {
        threadPool_.push_back(std::thread(&ProcessMgr::taskExecutor, &tasks_, &taskCv_,
            &taskLock_, &freeWorkers_));
    }
}

ProcessMgr::~ProcessMgr() {
    printf("ProcessMgr::~ProcessMgr\n");
    {
        std::unique_lock<std::mutex> scopeLock(taskLock_);
        // Send the shutdown signal to each task
        for (uint i = 0; i < numThreads_; ++i) {
            tasks_.push({ TaskType::DONE, 0 });
        }
        taskCv_.notify_all();
    }
    for (auto &thread : threadPool_) {
        thread.join();
    }
}

void ProcessMgr::taskExecutor(std::queue<Task> *tasks, std::condition_variable *cv, std::mutex *tl, uint *fw) {
    while (1) {
        std::unique_lock<std::mutex> scopeLock(*tl);
        // Check for any tasks in the queue
        cv->wait(scopeLock, [tasks] { return !tasks->empty(); });
        // Pull task and unlock
        auto task = tasks->front();
        tasks->pop();
        fw--;  // Reduce the number of free workers
        scopeLock.unlock();

        switch (task.type) {
            case TaskType::FUNC:
                // Do the actual task
                task.func();
                break;
            case TaskType::DONE:
                // Kill the task
                scopeLock.lock();
                fw++;
                cv->notify_all();
                return;
            default:
                fprintf(stderr, "ProcessMgr::taskExecutor: Unknown task type %d\n",
                    static_cast<int>(task.type));
                break;
        }

        // Signal this worker is now free
        scopeLock.lock();
        fw++;
        cv->notify_all();
    }
}

void ProcessMgr::sendTask(std::function<void(void)> taskFn) {
    std::unique_lock<std::mutex> scopeLock(taskLock_);
    tasks_.push({ TaskType::FUNC, taskFn });
    taskCv_.notify_all();
}

void ProcessMgr::waitComplete() {
    std::unique_lock<std::mutex> scopeLock(taskLock_);
    taskCv_.wait(scopeLock, [this] { return freeWorkers_ == numThreads_ && tasks_.empty(); });
}
