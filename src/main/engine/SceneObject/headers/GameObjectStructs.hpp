/**
 * @file GameObjectStructs.hpp
 * @author your name (you@domain.com)
 * @brief Structs used to instantiate GameObjects and colliders are defined here
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <string>
/// @todo - Change naming from [...]Structs to [...]Misc

/**
 * @brief Stores info about a GameObject's internal collider object.
 * @param offset(vec4) The distance between the center of the collider and its edges
 *    on the X, Y and Z axis.
 * @param minPoints(vec4) Contains the minimum points used in the automatically
 *    generated box collider for a GameObject.
 * @param center(vec4) The X, Y and Z coordinates of the center of the
 *    collider.
 * @param originalCenter(vec4) The raw X, Y and Z points for the center of the
 *    actual model. These points themselves should not be used for calculating
 *    collision.
 * @param collider(polygon*) The polygon data for the box collider drawn around a
 *    GameObject it is attached to.
 */
typedef struct colliderInfo {
    vec4 offset, minPoints;
    vec4 center, originalCenter;
    Polygon collider;
} colliderInfo;

