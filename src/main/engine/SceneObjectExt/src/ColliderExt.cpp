#include "ColliderObject.hpp"
#include <ColliderExt.hpp>

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
    // Extract colliders from c1 and c2
    auto co1 = dynamic_cast<ColliderObject *>(c1);
    auto co2 = dynamic_cast<ColliderObject *>(c2);
    return ColliderObject::getCollisionRaw(p1, co1, p2, co2);
}

void ColliderExt::updateCollider() {
    if (collider_.get() != nullptr) collider_->updateCollider();
}
