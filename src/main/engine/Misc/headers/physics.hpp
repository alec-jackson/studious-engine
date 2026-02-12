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
#include <mutex> //NOLINT
#include <shared_mutex> //NOLINT
#include <thread> //NOLINT
#include <queue>
#include <map>
#include <condition_variable> //NOLINT
#include <atomic>
#include <memory>
#include <functional>
#include <SceneObject.hpp>
#include <ColliderExt.hpp>
#include <glm/fwd.hpp>
#include <studious_utility.hpp>

#define SUBSCRIPTION_PARAM std::function<PhysicsReport *(void)>
#define PHYS_MAX_THREADS 256
#define PHYS_TRACE 1
#define MAX_PHYSICS_UPDATE_TIME 10.0f
#define CAP_TIME(ucTime) ucTime > MAX_PHYSICS_UPDATE_TIME ? MAX_PHYSICS_UPDATE_TIME : ucTime
#ifndef PHYS_THREADS
// Default thread count when not defined
#define PHYS_THREADS 1
#endif
#define GRAVITY_CONST 9.81f

#define GRAVITY_ACC_KEY "GRAVITY"
#define COLL_VEL_KEY "COLLISION"

enum PhysicsWorkType {
    POSITION,
    COLLISION,
    FINALIZE,
    SUBMIT,
    DIE
};

enum class PhysicsKinType {
    VELOCITY,
    ACCELERATION
};

struct PhysicsKinData {
    vec3 kinVec;
    PhysicsKinType kinType;
    float maxTime;
    float currentTime = 0.0f;
    vec3 calculatePos();
    vec3 calculateVel();
    vec3 reset();
    void updateTime(float time);
    bool isDone();
    inline explicit PhysicsKinData(vec3 kv, PhysicsKinType kt, float mt = -1.0f) : kinVec { kv }, kinType { kt },
        maxTime { mt } {}
};

// Internal - used in physics component
class PhysicsObject {
 public:
    SceneObject *        target;
    ColliderExt *        targetCollider;
    vec3                 position;
    vec3                 prevPos;
    vec3                 positionDelta = vec3(0.0f);
    vec3                 velocity;
    vec3                 velocityDelta = vec3(0.0f);
    map<string, SHD(PhysicsKinData)> kinTransforms;
    bool                 hasCollision = false;
    vec3                 acceleration;
    vec3                 jerk;
    bool                 isKinematic;
    bool                 obeyGravity;
    vec3                 impulse;
    float                elasticity;
    float                mass;
    double               runningTime;
    double               gravTime;
    PhysicsWorkType      workType;  // Might want to move this to a work queue specific class...
    std::mutex           objLock;
    /**
     * @brief Updates the position of the target object using the position formula.
     */
    void updatePosition();
    void wipeAllTransforms();

    void updateCollision(const map<string, std::shared_ptr<PhysicsObject>> &objects);
    void updateFinalize();
};

struct PhysicsParams {
    bool                isKinematic;
    bool                obeyGravity;
    float               elasticity;
    float               mass;
};

// External - published to subscribers
struct PhysicsReport {
    SceneObject *parentObject;
    vector<SceneObject *> collisions;
};

struct PhysicsSubscriber {
    inline PhysicsSubscriber(string n, SUBSCRIPTION_PARAM cb) : name(n), callback(cb) {}
    string name;
    SUBSCRIPTION_PARAM callback;
};

enum class PhysicsResult {
    OK,
    FAILURE,
    SHUTDOWN,
    REPEAT_NEEDED
};

float basicPhysics(float* pos, float fallspeed);

class PhysicsController {
 public:
    /**
     * @brief Creates a new physics controller with a given number of worker threads.
     * @param threadNum Number of worker threads to create for the PhysicsController.
     */
    explicit PhysicsController(uint threadNum);
    /**
     * @brief Adds a SceneObject to the PhysicsController for it to operate on.
     * @param object Target object for physics controller to update.
     * @param params Physical attributes of object being added to PhysicsController.
     * @return PhysicsResult::OK when object added successfully.
     */
    PhysicsResult addSceneObject(SceneObject *object, PhysicsParams params);
    /**
     * @brief Removes a scene object from the physics controller.
     * @param objectName Name of SceneObject to remove from PhysicsController.
     * @return PhysicsResult::OK when object is discovered and removed. PhysicsObject::FAILURE when object is
     * not discovered, so nothing happens.
     */
    PhysicsResult removeSceneObject(string objectName);
    /**
     * @brief Fetches a physics object from the PhysicsController. This is not thread safe, and is designed to
     * only be used in unit tests.
     * @param objectName Object to fetch from the PhysicsController.
     * @return shared pointer containing the discovered object if present. Invalid shared pointer is returned when
     * the object is not discovered.
     */
    std::shared_ptr<PhysicsObject> getPhysicsObject(string objectName);
    /**
     * @brief Sets the reference position of a SceneObject in the PhysicsController.
     * @param objectName SceneObject in the PhysicsController to set the reference position to.
     * @param position New reference position to set for the SceneObject.
     * @return PhysicsResult::OK when an object is discovered and has its position set. Otherwise,
     * PhysicsResult::FAILURE is returned and no objects are changed.
     */
    PhysicsResult setPosition(string objectName, vec3 position);
    PhysicsResult setVelocity(string objectName, string kinName, vec3 velocity);
    PhysicsResult setAcceleration(string objectName, string kinName, vec3 acceleration);
    PhysicsResult applyForce(string objectName, string kinName, vec3 force, float maxTime = -1.0f);
    PhysicsResult translate(string objectName, vec3 direction);
    PhysicsResult schedulePosition();
    PhysicsResult scheduleCollision();
    PhysicsResult scheduleFinalize();
    inline bool isPipelineComplete() { return workQueue_.empty() && freeWorkers_ == threadNum_; }
    PhysicsResult waitPipelineComplete();
    void update();
    PhysicsResult doWork();
    PhysicsResult shutdown();
    inline int hasShutdown() { return shutdown_; }
    inline const map<string, std::shared_ptr<PhysicsObject>> &getPhysicsObjects() { return physicsObjects_; }
    ~PhysicsController();
    static uint getDefaultThreadSize();

 private:
    std::atomic<uint> threadNum_;
    int shutdown_ = 0;
    std::vector<std::thread> threads_;
    std::shared_mutex physicsObjectQueueLock_;
    std::mutex subscriberLock_;
    std::mutex workQueueLock_;
    std::mutex controllerLock_;
    std::atomic<uint> freeWorkers_;
    map<string, std::shared_ptr<PhysicsObject>> physicsObjects_;
    queue<std::shared_ptr<PhysicsObject>> workQueue_;
    vector<PhysicsSubscriber> subscribers_;
    std::condition_variable workAvailableSignal_;
    std::condition_variable workCompletedSignal_;
    PhysicsResult addKinematicData_(string objectName, string kinName, vec3 kv, PhysicsKinType kt, float mt = -1.0f);
};
