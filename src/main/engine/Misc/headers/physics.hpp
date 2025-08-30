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
#define PHYS_TRACE 0
#ifndef PHYS_THREADS
// Default thread count when not defined
#define PHYS_THREADS 1
#endif

enum PhysicsWorkType {
    POSITION,
    COLLISION,
    FINALIZE,
    SUBMIT,
    DIE
};

class PhysicsForce {
 public:
    vec3 getForceForTime(float time);
    inline bool isDone() { return currentTime_ >= duration_; }
    inline PhysicsForce(vec3 force, float duration) :
        force_ { force }, duration_ { duration }, currentTime_ { 0.0f } {}
 private:
    vec3 force_;
    float duration_;
    float currentTime_;
    // Maybe we can add callbacks here that run when a force is complete?

};

class PhysicsForces {
 public:
    vec3 getCumulativeForceForTime(float time);
    void addWork(vec3 force, float duration);
    void clearForces();
 private:
    std::vector<PhysicsForce> forces_;
    std::mutex forcesLock_;
};

// Internal - used in physics component
class PhysicsObject {
 public:
    SceneObject *        target;
    vec3                 position;
    vec3                 velocity;
    vec3                 acceleration;
    vec3                 lastForce;
    vec3                 jerk;
    bool                 isKinematic;
    bool                 obeyGravity;
    vec3                 impulse;
    float                elasticity;
    float                mass;
    double               runningTime;
    PhysicsWorkType      workType;  // Might want to move this to a work queue specific class...
    PhysicsForces        forces;
    /**
     * @brief Updates the position of the target object using the position formula.
     */
    void basePosUpdate();
    /**
     * @brief Resets the reference position to the object's real position to allow the runningTime
     * counter to be reset without moving the object backwards.
     */
    void flushPosition();
    /**
     * @brief Updates the reference velocity to velocity + acceleration * runningTime to maintain
     * momentum of objects before a runningTime reset.
     */
    void flushVelocity();
    /**
     * @brief Does nothing for now, but will be useful when jerk is implemented.
     */
    void flushAcceleration();
    /**
     * @brief Runs position, velocity, acceleration flushes and resets the runningTime counter back
     * to zero.
     */
    void fullFlush();
    void updateAcceleration();
    void addWork(vec3 force, float duration);
 private:
};

struct PhysicsParams {
    bool                isKinematic;
    bool                obeyGravity;
    float               elasticity;
    float               mass;
    inline PhysicsParams(bool isKinematic, bool obeyGravity, float elasticity, float mass) :
        isKinematic { isKinematic }, obeyGravity { obeyGravity }, elasticity { elasticity }, mass { mass } {}
};

// External - published to subscribers
struct PhysicsReport {
    SceneObject *parentObject;
    vector<SceneObject *> collisions;
};

struct PhysicsSubscriber {
    inline PhysicsSubscriber(string name, SUBSCRIPTION_PARAM) : name(name), callback(callback) {}
    string name;
    SUBSCRIPTION_PARAM;
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
    explicit PhysicsController(int threadNum);
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
    PhysicsResult setVelocity(string objectName, vec3 velocity);
    PhysicsResult setAcceleration(string objectName, vec3 acceleration);
    PhysicsResult applyForce(string objectName, vec3 force);
    PhysicsResult applyWork(string objectName, vec3 force, float duration);
    PhysicsResult translate(string objectName, vec3 direction);
    PhysicsResult updatePosition();
    inline bool isPipelineComplete() { return workQueue_.empty() && freeWorkers_ == threadNum_; }
    PhysicsResult waitPipelineComplete();
    void update();
    PhysicsResult doWork();
    PhysicsResult shutdown();
    inline int hasShutdown() { return shutdown_; }
    inline const map<string, std::shared_ptr<PhysicsObject>> &getPhysicsObjects() { return physicsObjects_; }
    ~PhysicsController();

 private:
    std::atomic<int> threadNum_;
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
