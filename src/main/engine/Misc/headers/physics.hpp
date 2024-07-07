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
#include <GameObject.hpp>
#include <SafeQueue.hpp>

#define SUBSCRIPTION_PARAM void(*callback)(PhysicsReport*)  // NOLINT
#define PHYS_MAX_THREADS 256

static int PHYSICS_TRACE = 0;

// Internal - used in physics component
typedef struct PhysicsObject {
    GameObject *         gameObject;
    vector<float>        distance;
    vector<float>        velocity;
    vector<float>        acceleration;
    bool                 isKinematic;
    bool                 obeyGravity;
    vector<float>        impulse;
    float                elasticity;
} PhysicsObject;

typedef struct PhysicsParams {
    vector<float>       position;
    bool                isKinematic;
    bool                obeyGravity;
    float               elasticity;
} PhysicsParams;

// External - published to subscribers
typedef struct PhysicsReport {
    GameObject *parentObject;
    vector<GameObject *> collisions;
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
    PhysicsController(int threadNum);
    PhysicsResult addGameObject(GameObject *, PhysicsParams params);
    PhysicsResult removeGameObject(GameObject *);
    PhysicsResult subscribe(string name, SUBSCRIPTION_PARAM);
    PhysicsResult unsubscribe(string name);
    //static PhysicsResult setTrace(int trace);
 private:
    const int threadNum_;
    std::vector<std::thread> threads_;
    std::mutex objectLock_;
    vector<PhysicsObject *> physicsObjects_;
    vector<PhysicsSubscriber> subscribers_;
};
