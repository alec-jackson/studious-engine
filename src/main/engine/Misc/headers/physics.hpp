/**
 * @file physics.hpp
 * @author Alec Jackson
 * @brief Basic physics controller for GameObjects
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <thread>
#include <queue>
#include <condition_variable>
#include <atomic>
#include <SceneObject.hpp>

#define SUBSCRIPTION_PARAM void(*callback)(PhysicsReport*)  // NOLINT
#define PHYS_MAX_THREADS 256

//static int PHYSICS_TRACE = 0;

enum PhysicsWorkType {
    POSITION,
    COLLISION,
    FINALIZE,
    SUBMIT,
    DIE
};

// Internal - used in physics component
typedef struct PhysicsObject {
    SceneObject *        target;
    vector<float>        position;
    vector<float>        velocity;
    vector<float>        acceleration;
    bool                 isKinematic;
    bool                 obeyGravity;
    vector<float>        impulse;
    float                elasticity;
    float                mass;
    PhysicsWorkType      workType;  // MIght want to move this to a work queue specific class...
} PhysicsObject;

typedef struct PhysicsParams {
    vector<float>       position;
    bool                isKinematic;
    bool                obeyGravity;
    float               elasticity;
} PhysicsParams;

// External - published to subscribers
typedef struct PhysicsReport {
    SceneObject *parentObject;
    vector<SceneObject *> collisions;
} PhysicsReport;

typedef struct PhysicsSubscriber {
    inline PhysicsSubscriber(string name, SUBSCRIPTION_PARAM) : name(name), callback(callback) {}
    string name;
    SUBSCRIPTION_PARAM;
} PhysicsSubscriber;

enum PhysicsResult {
    PHYS_OK,
    PHYS_FAILURE
};

float basicPhysics(float* pos, float fallspeed);

class PhysicsController {
 public:
    explicit PhysicsController(int threadNum);
    PhysicsResult addSceneObject(SceneObject *, PhysicsParams params);
    PhysicsResult removeSceneObject(SceneObject *);
    PhysicsResult subscribe(string name, SUBSCRIPTION_PARAM);
    PhysicsResult unsubscribe(string name);
    /// @todo WATCH - need to make sure we're handling memory correctly here...
    PhysicsResult notifySubscribers(PhysicsReport *rep);
    PhysicsResult physicsScheduler();
    PhysicsResult doWork();
    PhysicsResult shutdown();
    inline int hasShutdown() { return shutdown_; }
    ~PhysicsController();
 private:
    const int threadNum_;
    int shutdown_ = 0;
    std::vector<std::thread> threads_;
    std::mutex objectLock_;
    std::mutex subscriberLock_;
    std::mutex workQueueLock_;
    std::atomic<int> freeWorkers_;
    vector<PhysicsObject *> physicsObjects_;
    queue<PhysicsObject *> workQueue_;
    vector<PhysicsSubscriber> subscribers_;
    std::condition_variable workAvailableSignal_;
    std::condition_variable workCompletedSignal_;
};
