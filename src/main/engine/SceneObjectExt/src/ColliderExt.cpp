/**
 * @file ColliderExt.cpp
 * @author Christian Galvez
 * @date 11-02-2025
 * @brief Implementation for the Collider Extension for SceneObjects.
 * @copyright Studious Engine 2025
 */
#include <ColliderExt.hpp>
#include <ColliderObject.hpp>

ColliderObject *ColliderExt::getCollider() {
    auto res = collider_.get();
    // So, we don't actually want to update the collider here. When run in parallel, there are writes
    // happening to the center_ mid-read.
    return res;
}

int ColliderExt::getCollision(ColliderExt *other) {
    return collider_->getCollision(other->getCollider());
}

int ColliderExt::getCollisionRaw(vec3 p1, ColliderExt *c1, vec3 p2, ColliderExt *c2) {
    return ColliderObject::getCollisionRaw(p1, c1->getCollider(), p2, c2->getCollider());
}

vec3 ColliderExt::getEdgePointRaw(vec3 p1, ColliderObject *c1, vec3 p2, ColliderObject *c2, vec3 epSign) {
    return ColliderObject::getEdgePointRaw(p1, c1, p2, c2, epSign);
}

void ColliderExt::updateCollider() {
    if (collider_.get() != nullptr) collider_->updateCollider();
}
