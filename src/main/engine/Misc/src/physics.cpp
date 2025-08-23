/**
 * @file physics.cpp
 * @author Alec Jackson
 * @brief Basic physics controller to apply to GameObjects in a GameInstance
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <physics.hpp>
#include <string>
#include <algorithm>
#include <condition_variable>

extern double deltaTime;

void PhysicsObject::basePosUpdate() {
    runningTime += deltaTime;
    vec3 pos;
    // Acceleration
    pos = vec3(0.5f) * acceleration * vec3(runningTime * runningTime);
    // Velocity
    pos += (velocity * vec3(runningTime));
    // Position
    pos += position;

    // Update the position of the target object
    target->setPosition(pos);
    printf("PhysicsObject::basePosUpdate: Updated position is %f, %f, %f\n", pos.x, pos.y, pos.z);
}

// Sleep the thread on the work safequeue until work becomes available
PhysicsResult PhysicsController::doWork() {
    while (1) {
        printf("physDoWork: Waiting for work\n");
        // Fetch work from the work queue if present
        std::unique_lock <std::mutex> scopeLock(workQueueLock_);
        workAvailableSignal_.wait(scopeLock, [this] () { return !workQueue_.empty(); });
        assert(!workQueue_.empty());
        auto physObj = workQueue_.front();
        workQueue_.pop();
        freeWorkers_ -= 1;
        assert(freeWorkers_ >= 0);
        scopeLock.unlock();  // No longer need lock after pulling work from queue
        if (physObj->workType == PhysicsWorkType::DIE) {
            // Close the thread
            printf("PhysicsController::doWork: Closing on DIE message\n");
            break;
        }
        string name = physObj->target->getObjectName();
        // Position function defined here...
        /**         1    2
         *  D(t) =  _ a t  + v t + x
         *          2
         */
        // Use OpenCL to speed this up maybe? - move this to PhysicsObject as a method

        printf("physDoWork: Retrieved gameObject for work [%s], work type [%d]\n", name.c_str(), physObj->workType);
        switch (physObj->workType) {
            case PhysicsWorkType::POSITION:
                physObj->basePosUpdate();
                break;
            default:
                printf("HORRIBLE BADNESS\n");
                break;
        }
        printf("physDoWork: Finished work [%s], work type [%d]\n", name.c_str(), physObj->workType);
        freeWorkers_ += 1;
        workCompletedSignal_.notify_one();
    }
    // Might not be necessary, but doing this to be safe for now
    freeWorkers_ += 1;
    assert(freeWorkers_ <= threadNum_);
    return PhysicsResult::OK;
}

// Takes a pointer to the y postion the current fallspeed, returns the updated fallspeed
// NOTE: Not a class function, can be called from a class with the y position variable
// Application: once a tick loop through all physics objects in the world and apply physics
// Should be updated to allow for setting the floor height
float basicPhysics(float* pos, float fallspeed) {
    if (*pos == 0 && fallspeed > 0)
        return 0;
    else if (fallspeed < .005f)
        fallspeed += 0.0001f;
    if (*pos > fallspeed || fallspeed < 0) {
        *pos -= fallspeed;
    } else if (*pos > 0) {
        *pos = 0;
    }
    return fallspeed;
}

/* Physics Proposal

TLDR; should somewhat resemble reality

* The physics class has access to a list of GameObjects with physics enabled.
* Objects can choose to obey gravity.
* Position of objects is changed via this physics component.
* Objects will have distance/velocity/acceleration components.
* The physics class will report all collisions to a queue.
* Objects can be either kinematic or passive. Kinematic objects can collide with one another.
* All kinematic objects MUST have mass (kg).
* When two objects collide, impulse will be calculated normally.
* For fun, I'll add an elasticity component to objects. This should let balls bounce :)
* Objects can be immovable - this means they will not partake in impulse calculations.
* Physics for object collision will be handled in this controller. Collision state changes will be reported
  via a physics queue.
* Components that want to listen to physics reports will subscribe to this component.
* The physics queue will propagate messages to any "subscribers".
* Collision processing MUST BE multithreaded. No exceptions.
*/

/**
 * @brief Adds a GameObject to the physics controller list
 *
 * @param gameObject to add to the list
 * @return PhysicsResult returns PHYS_OK
 */
PhysicsResult PhysicsController::addSceneObject(SceneObject *sceneObject, PhysicsParams params) {
    // Retrieve the exclusive lock for the game object list
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);

    // Create a new physics profile for the object
    auto physicsObject = std::make_shared<PhysicsObject>();
    auto poPtr = physicsObject.get();
    poPtr->target = sceneObject;
    poPtr->position = sceneObject->getPosition();
    poPtr->velocity = {0.0f, 0.0f, 0.0f};
    poPtr->acceleration = {0.0f, 0.0f, 0.0f};
    poPtr->isKinematic = params.isKinematic;
    poPtr->obeyGravity = params.obeyGravity;
    poPtr->impulse = {0.0f, 0.0f, 0.0f};
    poPtr->elasticity = params.elasticity;
    poPtr->mass = params.mass;
    poPtr->runningTime = 0.0;

    // Add the object to the physics object list
    assert(!sceneObject->getObjectName().empty());
    physicsObjects_[sceneObject->getObjectName()] = physicsObject;
    return PhysicsResult::OK;
}

PhysicsResult PhysicsController::removeSceneObject(string objectName) {
    auto res = PhysicsResult::OK;
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    auto poit = physicsObjects_.find(objectName);
    if (poit != physicsObjects_.end()) {
        printf("PhysicsController::removeGameObject: Deleting object %s\n", objectName.c_str());
        physicsObjects_.erase(poit);
    } else {
        fprintf(stderr,
            "PhysicsController::removeSceneObject: %s is not present in the physics controller!\n",
        objectName.c_str());
        res = PhysicsResult::FAILURE;
    }
    return res;
}

std::shared_ptr<PhysicsObject> PhysicsController::getPhysicsObject(string objectName) {
    auto poit = physicsObjects_.find(objectName);
    std::shared_ptr<PhysicsObject> res;
    if (poit != physicsObjects_.end()) {
        res = poit->second;
    } else {
        fprintf(stderr,
            "PhysicsController::getPhysicsObject: %s does not exist in phys controller\n",
            objectName.c_str());
    }
    return res;
}

PhysicsController::PhysicsController(int threadNum) : threadNum_{threadNum} {
    printf("PhysicsController::PhysicsController: Entered constructor\n");
    if (threadNum_ > PHYS_MAX_THREADS) return;
    // Set the initial free workers to threadNum
    freeWorkers_ = threadNum;
    // Create thread pool for physics calculations
    for (int i = 0; i < threadNum_; ++i) {
        threads_.emplace_back(&PhysicsController::doWork, this);
    }
    printf("PhysicsController::PhysicsController: Exit\n");
}

PhysicsController::~PhysicsController() {
    std::unique_lock<std::mutex> scopeLock(workQueueLock_);
    printf("PhysicsController::~PhysicsController\n");
    shutdown();  // Mark the scheduler to shutdown
    // Thread safety for this variable probably isn't super important...
    auto deathMsg = std::make_shared<PhysicsObject>();
    deathMsg.get()->workType = PhysicsWorkType::DIE;
    // When we end, send kill signal to threads and join
    for (int i = 0; i < threadNum_; ++i) {
        printf("Sending kill to worker queue...\n");
        workQueue_.push(deathMsg);
    }
    workAvailableSignal_.notify_all();
    scopeLock.unlock();
    for (auto &thread : threads_) {
        printf("Joining worker thread...\n");
        thread.join();
    }
}

/**
 * @brief Runs the physics scheduler for the physics controller.
 * The physics pipeline has four stages: position, collision, finalize, submit. Each phase can be parallelized
 * when run on its own, but multiple different stages in the pipeline should NOT be run simultaneously...
 *
 * POSITION - This is the first step in the physics pipeline. At the start, just feed all of the physics objects
 * into the work queue to start. This will update the positions of each gameobject. This considers the object's
 * force (acceleration * mass), as well as other things.
 *
 * COLLISION - This is the second step in the pipeline. After the positions of all objects has been updated, we can
 * start checking for collisions with each object. This is going to be a very heavy step. We're going to check for
 * collisions against all of the other objects in the scene. This can be optimized using object distances later, but for
 * V1 this is OK. We check for collisions and then report any collisions via physics reports. Subscribers to physics events
 * will be notified.
 *
 * FINALIZE - This stage is going to handle the physics behind object collisions between two objects, We can calculate
 * impulse or whatever else we want here, and then update the object's acceleration/velocity/position again. When
 * objects collide here, we want to send those objects BACK into the POSITION workload. This will also require checking
 * for COLLISIONS again, and then finalizing again... We can run into infiite loops here if we're possible, but again
 * that's a V2 issue :)
 *
 * SUBMIT - This may not actually be a separate step, but we'll need to see how this turns out. We want to make sure we
 * report all of the physics events to their subscribers. Maybe we'll only submit physics reports at this step?
 *
 * @return PhysicsResult
 */
PhysicsResult PhysicsController::updatePosition() {
    printf("PhysicsController::updatePosition: Enter\n");
    if (shutdown_) return PhysicsResult::SHUTDOWN;
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    workQueueLock_.lock();
    // Run the initial POSITION pipeline step here with all objects - maybe check for kinematic
    for (auto physObjEntry : physicsObjects_) {
        physObjEntry.second.get()->workType = PhysicsWorkType::POSITION;
        workQueue_.push(physObjEntry.second);
    }
    workQueueLock_.unlock();
    workAvailableSignal_.notify_all();
    return PhysicsResult::OK;
}

PhysicsResult PhysicsController::waitPipelineComplete() {
    std::unique_lock<std::mutex> scopeLock(workQueueLock_);
    workCompletedSignal_.wait(scopeLock, [this]() { return isPipelineComplete() || shutdown_; });
    return shutdown_ ? PhysicsResult::SHUTDOWN : PhysicsResult::OK;
}

void PhysicsController::update() {
    // Stop updating when shutdown received
    // Physics pipeline updated here...
    updatePosition();
    waitPipelineComplete();
}

PhysicsResult PhysicsController::shutdown() {
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    printf("PhysicsController::shutdown: Sending shutdown signal\n");
    // Mark the shutdown variable as true
    shutdown_ = 1;
    return PhysicsResult::OK;
}

PhysicsResult PhysicsController::setPosition(string objectName, vec3 position) {
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    auto result = PhysicsResult::FAILURE;
    auto poit = physicsObjects_.find(objectName);
    if (poit != physicsObjects_.end()) {
        poit->second.get()->position = position;
        result = PhysicsResult::OK;
    } else {
        printf("PhysicsController::setPosition: %s not found", objectName.c_str());
    }
    return result;
}

PhysicsResult PhysicsController::setVelocity(string objectName, vec3 velocity) {
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    auto result = PhysicsResult::FAILURE;
    auto poit = physicsObjects_.find(objectName);
    if (poit != physicsObjects_.end()) {
        // On velocity change, flush object position and reset time
        poit->second.get()->flushPosition();
        poit->second.get()->resetTime();
        poit->second.get()->velocity = velocity;
        result = PhysicsResult::OK;
    } else {
        printf("PhysicsController::setVelocity: %s not found", objectName.c_str());
    }
    return result;
}

PhysicsResult PhysicsController::setAcceleration(string objectName, vec3 acceleration) {
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    auto result = PhysicsResult::FAILURE;
    auto poit = physicsObjects_.find(objectName);
    if (poit != physicsObjects_.end()) {
        // On acceleration change, flush object position and reset time
        poit->second.get()->flushPosition();
        poit->second.get()->resetTime();
        poit->second.get()->acceleration = acceleration;
        result = PhysicsResult::OK;
    } else {
        printf("PhysicsController::setAcceleration: %s not found", objectName.c_str());
    }
    return result;
}

PhysicsResult PhysicsController::setForce(string objectName, vec3 force) {
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    auto result = PhysicsResult::FAILURE;
    auto poit = physicsObjects_.find(objectName);
    if (poit != physicsObjects_.end()) {
        poit->second.get()->force = force;
        result = PhysicsResult::OK;
    } else {
        printf("PhysicsController::setAcceleration: %s not found", objectName.c_str());
    }
    return result;
}

PhysicsResult PhysicsController::translate(string objectName, vec3 translation) {
    std::unique_lock<std::mutex> scopeLock(physicsObjectQueueLock_);
    auto result = PhysicsResult::FAILURE;
    auto poit = physicsObjects_.find(objectName);
    if (poit != physicsObjects_.end()) {
        poit->second.get()->position += translation;
        result = PhysicsResult::OK;
    } else {
        printf("PhysicsController::translate: %s not found", objectName.c_str());
    }
    return result;
}
