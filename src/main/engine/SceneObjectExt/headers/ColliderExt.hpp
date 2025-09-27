#pragma once
#include <ColliderObject.hpp>

class ColliderExt {
 public:
    ColliderObject *getCollider();
    virtual void createCollider() = 0;
 protected:
    std::shared_ptr<ColliderObject> collider_;
};
