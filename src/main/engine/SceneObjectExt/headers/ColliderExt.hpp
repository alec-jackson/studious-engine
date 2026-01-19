/**
 * @file ColliderExt.hpp
 * @author Christian Galvez
 * @date 11-02-2025
 * @brief Declaration for Collider Extension for Scene Objects.
 * @copyright Studious Engine 2025
 */
#pragma once
#include <memory>
#include <ColliderObject.hpp>
#include <studious_utility.hpp>

class ColliderExt {
 public:
    VEC(SHD(ColliderObject)) getColliders();
    int getCollision(SHD(ColliderExt) other);
    void updateCollider();
    virtual void createCollider() = 0;
    static int getCollisionRaw(vec3 p1, SHD(ColliderExt) c1, vec3 p2, SHD(ColliderExt) c2);
 protected:
    VEC(SHD(ColliderObject)) colliders_;
};
