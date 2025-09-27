#include <ColliderExt.hpp>

ColliderObject *ColliderExt::getCollider() {
    assert(collider_.get() != nullptr);
    collider_.get()->updateCollider();
    return collider_.get();
}
