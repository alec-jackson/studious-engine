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
#include <GameObject.hpp>

#define SUBSCRIPTION_PARAM void(*callback)(PhysicsReport*)  // NOLINT

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

// External - published to subscribers
typedef struct PhysicsReport {
    GameObject *parentObject;
    vector<GameObject *> collisions;
} PhysicsReport;

typedef struct PhysicsSubscriber {
    string name;
    SUBSCRIPTION_PARAM;
};

enum PhysicsResult {
    OK,
    FAILURE
};

float basicPhysics(float* pos, float fallspeed);

class PhysicsController {
 public:
    inline PhysicsController();
    PhysicsResult addGameObject(GameObject *);
    PhysicsResult removeGameObject(GameObject *);
    PhysicsResult subscribe(SUBSCRIPTION_PARAM, string name);
    static PhysicsResult setTrace(int trace);
    PhysicsResult calculate();
 private:
    vector<PhysicsObject *> physicsObjects_;
    vector<PhysicsSubscriber> subscribers_;
};
