#pragma once
#include <ColliderObject.hpp>

class ColliderExt {
 public:
    ColliderObject *getCollider();
    int getCollision(ColliderExt *other);
    void updateCollider();
    virtual void createCollider() = 0;
 protected:
    std::shared_ptr<ColliderObject> collider_;
};
