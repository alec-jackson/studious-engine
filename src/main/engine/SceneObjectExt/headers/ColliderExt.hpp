#pragma once
#include <ColliderObject.hpp>

class ColliderExt {
 public:
    ColliderObject *getCollider();
    int getCollision(ColliderExt *other);
    void updateCollider();
    virtual void createCollider() = 0;
    static int getCollisionRaw(vec3 p1, ColliderExt *c1, vec3 p2, ColliderExt *c2);
 protected:
    std::shared_ptr<ColliderObject> collider_;
};
