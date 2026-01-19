/**
 * @file ColliderExt.cpp
 * @author Christian Galvez
 * @date 11-02-2025
 * @brief Implementation for the Collider Extension for SceneObjects.
 * @copyright Studious Engine 2025
 */
#include <ColliderExt.hpp>
#include <ColliderObject.hpp>

VEC(SHD(ColliderObject)) ColliderExt::getColliders() {
    // So, we don't actually want to update the collider here. When run in parallel, there are writes
    // happening to the center_ mid-read.
    return colliders_;
}

int ColliderExt::getCollision(SHD(ColliderExt) other) {
    int res = 0; // No collision by default
    auto otherColliders = other->getColliders();
    for (auto &collider : colliders_) {
        // This is going to be n^2, but hopefully we can break early
        for (auto &oCollider : otherColliders) {
            int coll = collider->getCollision(oCollider);
            // Break early if a sub-collider is colliding
            if (coll != 0) {
                res = coll;
                break;
            }
        }
        if (res != 0) break;
    }
    // Sample collision across all subcolliders
    return res;
}

int ColliderExt::getCollisionRaw(vec3 p1, SHD(ColliderExt) c1, vec3 p2, SHD(ColliderExt) c2) {
    auto theseColliders = c1->getColliders();
    auto otherColliders = c2->getColliders();
    bool valid = !theseColliders.empty() && !otherColliders.empty();
    return valid ? ColliderObject::getCollisionRaw(p1, c1->getColliders().front(), p2, c2->getColliders().front()) : 0;
}

void ColliderExt::updateCollider() {
    for (auto &collider : colliders_) {
        collider->updateCollider();
    }
}
