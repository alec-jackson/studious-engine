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

static PhysicsResult physDoWork() {
    printf("This is a test\n");
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
PhysicsResult PhysicsController::addGameObject(GameObject *gameObject, PhysicsParams params) {
    // Retrieve the exclusive lock for the game object list
    std::unique_lock<std::mutex> scopeLock(objectLock_);

    // Create a new physics profile for the object
    auto physicsObject = new PhysicsObject();
    if (physicsObject == nullptr) return PhysicsResult::PHYS_FAILURE;
    physicsObject->gameObject = gameObject;
    physicsObject->distance = params.position;
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

PhysicsResult PhysicsController::removeGameObject(GameObject *gameObject) {
    auto compare = [&gameObject](PhysicsObject *po) {
        return gameObject->getObjectName().compare(po->gameObject->getObjectName()) == 0;
    };
    auto it = std::find_if(physicsObjects_.begin(), physicsObjects_.end(), compare);
    if (it != physicsObjects_.end()) {
        auto physObject = *it;
        printf("PhysicsController::removeGameObject: Deleting object %p\n", gameObject);
        physicsObjects_.erase(it);
        delete physObject;
    }
    return PhysicsResult::PHYS_OK;
}

PhysicsResult PhysicsController::subscribe(string name, SUBSCRIPTION_PARAM) {
    auto physicsSubscriber = PhysicsSubscriber(name, callback);
    subscribers_.push_back(physicsSubscriber);
    return PHYS_OK;
}

PhysicsResult PhysicsController::unsubscribe(string name) {
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
    if (threadNum_ > PHYS_MAX_THREADS) return;
    // Create thread pool for physics calculations
    for (int i = 0; i < threadNum_; ++i) {
        threads_.emplace_back(physDoWork);
    }
    for (auto i = threads_.begin(); i != threads_.end(); i++) {
        i->join();
    }
}

