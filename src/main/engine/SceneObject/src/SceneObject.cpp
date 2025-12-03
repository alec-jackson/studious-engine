/**
 * @author Christian Galvez
 * @date 2025-07-12
 * @brief Implementation of some SceneObject methods.
 * @copyright Copyright (c) 2025
 */
#include <SceneObject.hpp>
#include <cstdio>
#include <algorithm>

void SceneObject::updateModelMatrices() {
    vec3 pos = getPosition();
    vec3 rot = getRotation();
    float scale = getScale();
    // Update translation
    translateMatrix_ = glm::translate(mat4(1.0f), pos);
    rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rot[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rot[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rot[2]),
            vec3(0, 0, 1));
    scaleMatrix_ = glm::scale(mat4(1.0f), vec3(scale));
}

void SceneObject::addChild(SceneObject *child) {
    if (nullptr == child) {
        fprintf(stderr, "SceneObject::addChild: Passed in child is null!\n");
        return;
    }
    // Add a child to the children list
    children_.insert(child);
    // Update the child's parent
    child->parent_ = this;
    // finalize the parent
    finalize();
}

void SceneObject::setParent(SceneObject *parent) {
    // Update this object's parent
    parent_ = parent;
    if (nullptr != parent) {
        // Add this object as the parent's child
        parent_->addChild(this);
    }
    // finalize the child
    finalize();
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
            child->objectName().c_str(),
            this->objectName().c_str());
    }
}

void SceneObject::shiftRenderPriorityBy(int change) {
    renderPriority_ += change;
    renderPriority_ = std::min(renderPriority_, RENDER_PRIOR_HIGHEST);
}

void SceneObject::modifyPosition(vec3 pos) {
    position += pos;
}

void SceneObject::modifyRotation(vec3 rot) {
    rotation += rot;
}

void SceneObject::modifyScale(float scale) {
    scale_ += scale;
}
