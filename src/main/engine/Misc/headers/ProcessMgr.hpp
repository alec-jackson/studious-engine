/**
 * @file ProcessMgr.hpp
 * @author Christian Galvez
 * @date 02-23-26
 * @brief Class that can quickly handle processing bulk tasks across
 * multiple threads.
 * @copyright Studious Engine 2026
 */

#pragma once
#include <thread>  //NOLINT
#include <vector>
#include <condition_variable>  //NOLINT
#include <mutex>  //NOLINT
#include <queue>
#include <SceneObject.hpp>

enum class TaskType {
    FUNC,
    DONE
};

struct Task {
    TaskType type;
    std::function<void(void)> func;
};

/**
 * This class handles executing pools of tasks - users of this should
 * just be able to throw in function pointers as tasks, and convenience
 * methods will help synchronize and get statistics on those tasks.
 */

class ProcessMgr {
 public:
    explicit ProcessMgr(uint numThreads);
    ~ProcessMgr();
    void waitComplete();
    void sendTask(std::function<void(void)> taskFn);
    static void taskExecutor(std::queue<Task> *tasks, std::condition_variable *cv, std::mutex *tl, uint *fw);

 private:
    uint numThreads_;
    uint freeWorkers_;
    std::queue<Task> tasks_;
    std::vector<std::thread> threadPool_;
    std::condition_variable taskCv_;
    std::mutex taskLock_;
};
