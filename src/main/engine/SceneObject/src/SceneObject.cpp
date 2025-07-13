/**
 * @author Christian Galvez
 * @date 2025-07-12
 * @brief Implementation of some SceneObject methods.
 * @copyright Copyright (c) 2025
 */
#include <SceneObject.hpp>
#include <cstdio>

void SceneObject::updateModelMatrices() {
    vec3 pPos = vec3(0), pRot = vec3(0), pScale = vec3(0);
    if (parent_) {
        pPos = parent_->getPosition();
        pRot = parent_->getRotation();
        pScale = vec3(parent_->getScale());
    }
    // Update translation
    translateMatrix_ = glm::translate(mat4(1.0f), position + pPos);
    rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rotation[0] + pRot[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1] + pRot[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2] + pRot[2]),
            vec3(0, 0, 1));
    scaleMatrix_ = glm::scale(mat4(1.0f), vec3(scale_) + pScale);
}

void SceneObject::addChild(SceneObject *child) {
    if (nullptr == child) {
        fprintf(stderr, "SceneObject::addChild: Passed in child is null!\n");
        return;
    }
    // Add a child to the children list
    children_.push_back(child);
    // Update the child's parent
    child->parent_ = this;
}

void SceneObject::setParent(SceneObject *parent) {
    // Update this object's parent
    parent_ = parent;
    if (nullptr != parent) {
        // Add this object as the parent's child
        parent_->addChild(this);
    }
}

SceneObject::~SceneObject() {
    // When the destructor is called, unset parents/children
    if (parent_) {
        parent_->removeChild(this);
    }
    // Unset the parent pointer from all children
    for (auto obj : children_) {
        obj->setParent(nullptr);
    }
}

void SceneObject::removeChild(SceneObject *child) {
    // Remove the child object from this gameobject
    auto cit = std::find_if(children_.begin(), children_.end(), [child](SceneObject *obj) { return obj == child; });
    if (cit != children_.end()) {
        // If we find the child, delete it
        children_.erase(cit);
    } else {
        printf("SceneObject::removeChild: %s is not a child of %s\n",
            child->getObjectName().c_str(),
            this->getObjectName().c_str());
    }
}
