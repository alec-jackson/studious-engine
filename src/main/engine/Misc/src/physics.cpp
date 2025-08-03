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
        printf("physDoWork: Retrieved gameObject for work [%s], work type [%d]\n", name.c_str(), physObj->workType);
        switch (physObj->workType) {
            case PhysicsWorkType::POSITION:
                // Perform work here, determine if another iteration is required
                // Missing - ? Update Acceleration...
                // Update velocity from acceleration - might have a cleaner way to do this
                physObj->velocity[0] += physObj->acceleration[0];
                physObj->velocity[1] += physObj->acceleration[1];
                physObj->velocity[2] += physObj->acceleration[2];
                // Upate position from velocity
                physObj->position[0] += physObj->velocity[0];
                physObj->position[1] += physObj->velocity[1];
                physObj->position[2] += physObj->velocity[2];
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
    return PhysicsResult::PHYS_OK;
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
    std::unique_lock<std::mutex> scopeLock(objectLock_);

    // Create a new physics profile for the object
    auto physicsObject = new PhysicsObject();
    if (physicsObject == nullptr) return PhysicsResult::PHYS_FAILURE;
    physicsObject->target = sceneObject;
    physicsObject->position = params.position;
    physicsObject->velocity = {0.0f, 0.0f, 0.0f};
    physicsObject->acceleration = {0.0f, 0.0f, 0.0f};
    physicsObject->isKinematic = params.isKinematic;
    physicsObject->obeyGravity = params.obeyGravity;
    physicsObject->impulse = {0.0f, 0.0f, 0.0f};
    physicsObject->elasticity = 0.0f;

    // Add the object to the physics object list
    physicsObjects_.push_back(physicsObject);
    return PhysicsResult::PHYS_OK;
}

PhysicsResult PhysicsController::removeSceneObject(SceneObject *sceneObject) {
    std::unique_lock<std::mutex> scopeLock(objectLock_);
    auto compare = [&sceneObject](PhysicsObject *po) {
        return sceneObject->getObjectName().compare(po->target->getObjectName()) == 0;
    };
    auto it = std::find_if(physicsObjects_.begin(), physicsObjects_.end(), compare);
    if (it != physicsObjects_.end()) {
        auto physObject = *it;
        printf("PhysicsController::removeGameObject: Deleting object %s\n", sceneObject->getObjectName().c_str());
        physicsObjects_.erase(it);
        delete physObject;
    }
    return PhysicsResult::PHYS_OK;
}

PhysicsResult PhysicsController::subscribe(string name, SUBSCRIPTION_PARAM) {
    std::unique_lock<std::mutex> scopeLock(subscriberLock_);
    auto physicsSubscriber = PhysicsSubscriber(name, callback);
    subscribers_.push_back(physicsSubscriber);
    return PHYS_OK;
}

PhysicsResult PhysicsController::unsubscribe(string name) {
    std::unique_lock<std::mutex> scopeLock(subscriberLock_);
    auto compare = [&name](PhysicsSubscriber sub) {
        return name.compare(sub.name) == 0;
    };
    // Remove the subscriber from the subscriptions list
    auto it = std::find_if(subscribers_.begin(), subscribers_.end(), compare);
    if (it != subscribers_.end()) {
        printf("PhysicsController::unsubscribe: Removed subscription %s\n", it->name.c_str());
        subscribers_.erase(it);
    }
    return PHYS_OK;
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
    PhysicsObject death;
    death.workType = PhysicsWorkType::DIE;
    // When we end, send kill signal to threads and join
    for (int i = 0; i < threadNum_; ++i) {
        printf("Sending kill to worker queue...\n");
        workQueue_.push(&death);
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
PhysicsResult PhysicsController::physicsScheduler() {
    printf("PhysicsController::physicsScheduler: Start\n");
    auto pipelineStage = PhysicsWorkType::POSITION;  // Start the pipeline at the POSITION step
    printf("PhysicsController::physicsScheduler: About to obtain objectLock_\n");
    std::unique_lock<std::mutex> scopeLock(objectLock_);
    printf("PhysicsController::physicsScheduler: Obtained object lock\n");
    // Check if the shutdown signal was received
    if (shutdown_) return PHYS_OK;
    // Obtain the workQueue lock
    workQueueLock_.lock();
    printf("PhysicsController::physicsScheduler: Populating work queue\n");
    // Run the initial POSITION pipeline step here with all objects - maybe check for kinematic
    for (auto i = physicsObjects_.begin(); i != physicsObjects_.end(); i++) {
        (*i)->workType = pipelineStage;
        workQueue_.push(*i);
    }
    workQueueLock_.unlock();
    printf("PhysicsController::physicsScheduler: Populated work queue\n");
    workAvailableSignal_.notify_all();
    printf("PhysicsController::physicsScheduler: Completed initial\n");
    // Wait for all of the child threads to finish their work before scheduling more work
    workCompletedSignal_.wait(scopeLock, [this]() { return freeWorkers_ == threadNum_ && workQueue_.empty(); });
    printf("PhysicsController::physicsScheduler: Shutdown signal received\n");
    return PhysicsResult::PHYS_OK;
}

void PhysicsController::update() {
    physicsScheduler();
}

PhysicsResult PhysicsController::notifySubscribers(PhysicsReport *rep) {
    // We need to lock the subscriber list when notifying
    std::unique_lock<std::mutex> scopeLock(subscriberLock_);
    for (auto sub = subscribers_.begin(); sub != subscribers_.end(); sub++) {
        // Send the physics report to each registered callback
        sub->callback(rep);
    }
    return PHYS_OK;
}

PhysicsResult PhysicsController::shutdown() {
    std::unique_lock<std::mutex> scopeLock(objectLock_);
    printf("PhysicsController::shutdown: Sending shutdown signal\n");
    // Mark the shutdown variable as true
    shutdown_ = 1;
    return PhysicsResult::PHYS_OK;
}