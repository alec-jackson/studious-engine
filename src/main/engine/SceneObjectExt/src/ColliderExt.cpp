#include <ColliderExt.hpp>

ColliderObject *ColliderExt::getCollider() {
    auto res = collider_.get();
    if (nullptr != res) {
        res->updateCollider();
    }
    return res;
}
