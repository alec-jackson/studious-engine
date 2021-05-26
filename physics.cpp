#include "physics.hpp"
// Takes a pointer to the y postion the current fallspeed, returns the updated fallspeed
// NOTE: Not a class function, can be called from a class with the y position variable
// Application: once a tick loop through all physics objects in the world and apply physics
// Should be updated to allow for setting the floor height
float basicPhysics(float* pos, float fallspeed){
    if (*pos == 0 && fallspeed > 0)
        return 0;
    else if (fallspeed < .005f)
        fallspeed += 0.0001f;
    
    if (*pos > fallspeed || fallspeed < 0){
        *pos -= fallspeed;
    } else if (*pos > 0){
        *pos = 0;
    }
    return fallspeed;
}

