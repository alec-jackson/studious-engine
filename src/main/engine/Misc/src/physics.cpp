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
* Objects can be immovable - this means they will not partake in impulse calculations.
* Physics for object collision will be handled in this controller. Collision state changes will be reported
  via a physics queue that other components can access.
* Collision processing MUST BE multithreaded. No exceptions.
*/
