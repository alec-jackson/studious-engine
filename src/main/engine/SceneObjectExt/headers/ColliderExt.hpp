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

class ColliderExt {
 public:
    ColliderObject *getCollider();
    int getCollision(ColliderExt *other);
    void updateCollider();
    virtual void createCollider(string tag) = 0;
    static int getCollisionRaw(vec3 p1, ColliderExt *c1, vec3 p2, ColliderExt *c2);
    static vec3 getEdgePointRaw(vec3 p1, ColliderObject *c1, vec3 p2, ColliderObject *c2, vec3 epSign);
    inline vec3 getCenter() { return collider_->center(); }
 protected:
    std::shared_ptr<ColliderObject> collider_;
};
