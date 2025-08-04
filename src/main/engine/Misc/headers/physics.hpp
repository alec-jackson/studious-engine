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
#include <map>
#include <condition_variable>
#include <atomic>
#include <memory>
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
    vec3                 position;
    vec3                 velocity;
    vec3                 acceleration;
    vec3                 jerk;
    vec3                 force;
    bool                 isKinematic;
    bool                 obeyGravity;
    vec3                 impulse;
    float                elasticity;
    float                mass;
    PhysicsWorkType      workType;  // Might want to move this to a work queue specific class...
} PhysicsObject;

typedef struct PhysicsParams {
    bool                isKinematic;
    bool                obeyGravity;
    float               elasticity;
    float               mass;
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

enum class PhysicsResult {
    OK,
    FAILURE,
    SHUTDOWN,
    REPEAT_NEEDED
};

float basicPhysics(float* pos, float fallspeed);

class PhysicsController {
 public:
    explicit PhysicsController(int threadNum);
    PhysicsResult addSceneObject(SceneObject *, PhysicsParams params);
    PhysicsResult removeSceneObject(string objectName);
    std::shared_ptr<PhysicsObject> getPhysicsObject(string objectName);
    PhysicsResult updatePosition();
    inline bool isPipelineComplete() { return freeWorkers_ == threadNum_ && workQueue_.empty(); }
    PhysicsResult waitPipelineComplete();
    void update();
    PhysicsResult doWork();
    PhysicsResult shutdown();
    inline int hasShutdown() { return shutdown_; }
    inline const map<string, std::shared_ptr<PhysicsObject>> &getPhysicsObjects() { return physicsObjects_; }
    ~PhysicsController();
 private:
    const int threadNum_;
    int shutdown_ = 0;
    std::vector<std::thread> threads_;
    std::mutex physicsObjectQueueLock_;
    std::mutex subscriberLock_;
    std::mutex workQueueLock_;
    std::atomic<int> freeWorkers_;
    map<string, std::shared_ptr<PhysicsObject>> physicsObjects_;
    queue<std::shared_ptr<PhysicsObject>> workQueue_;
    vector<PhysicsSubscriber> subscribers_;
    std::condition_variable workAvailableSignal_;
    std::condition_variable workCompletedSignal_;
};
