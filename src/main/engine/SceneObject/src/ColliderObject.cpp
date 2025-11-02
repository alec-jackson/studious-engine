/**
 * @file ColliderObject.cpp
 * @author Christian Galvez
 * @brief
 * @version 0.1
 * @date 2024-02-15
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <ColliderObject.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <memory>

#define EPSILON 1e-6

/** @todo Update - this is the old struct info
 * @brief Stores info about a GameObject's internal collider object.
 * @param offset(vec4) The distance between the center of the collider and its edges
 *    on the X, Y and Z axis.
 * @param minPoints(vec4) Contains the minimum points used in the automatically
 *    generated box collider for a GameObject.
 * @param center(vec4) The X, Y and Z coordinates of the center of the
 *    collider.
 * @param originalCenter(vec4) The raw X, Y and Z points for the center of the
 *    actual model. These points themselves should not be used for calculating
 *    collision.
 * @param collider(polygon*) The polygon data for the box collider drawn around a
 *    GameObject it is attached to.
 */
ColliderObject::ColliderObject(std::shared_ptr<Polygon> target, uint programId, SceneObject *owner) :
    SceneObject(owner->type(), owner->objectName() + "-Collider", owner->gfxController()), target_ { target }, pTranslateMatrix_ { owner->translateMatrix() },
    pScaleMatrix_ { owner->scaleMatrix() }, pVpMatrix_ { owner->vpMatrix() }, pPos_ { owner->getPosition() } {
    programId_ = programId;
    createCollider();
}

/**
 * @brief Constructor for 2D collider objects.
 */
ColliderObject::ColliderObject(const vector<float> &vertTexData, unsigned int programId, SceneObject *owner) :
    SceneObject(owner->type(), owner->objectName() + "-Collider", owner->gfxController()), pTranslateMatrix_ { owner->translateMatrix() }, pScaleMatrix_ { owner->scaleMatrix() },
    pVpMatrix_ { owner->vpMatrix() }, pPos_ { owner->getPosition() } {
    programId_ = programId;
    // Separate vertex data from vertTexData
    assert(vertTexData.size() % 4 == 0);
    vector<float> vertices;
    for (uint i = 0; i < vertTexData.size(); ++i) {
        if (i % 4 == 3) continue;
        if (i % 4 == 2) {
            // Add a zero to the Z axis for 2D objects
            vertices.push_back(0.0);
            continue;
        }
        vertices.push_back(vertTexData.at(i));
    }
    target_ = std::make_shared<Polygon>(vertices.size(), vertices);
    createCollider();
}

void ColliderObject::updateCollider() {
    // Update center position with model matrix
    center_ = pTranslateMatrix_ * pScaleMatrix_ * originalCenter_;
    vec4 minOffset = pTranslateMatrix_ * pScaleMatrix_ * minPoints_;
    // Use rescaled edge points to calculate offset on the fly!
    offset_ = center_ - minOffset;
}

/**
 * @brief Checks if this collider is colliding with another collider.
 *
 * @param object other collider to check collision with
 * @return int -1 if error, 0 for no collision, 1 for colliding
 */
int ColliderObject::getCollision(ColliderObject *object) {
    // Center = critical section?
    int matching = 0;  // Number of axis that have collided
    if (object == nullptr) {
        cerr << "Error: Cannot get collision for NULL GameObjects!\n";
        return NO_MATCH;
    }
    auto delta = object->center() - this->center();
    auto range = object->offset() + this->offset();
    for (uint i = 0; i < 3; ++i) {
        float res = range[i] - abs(delta[i]);
        if (range[i] > abs(delta[i]) && abs(res) > EPSILON) {
            matching |= (1<<i);
        }
    }

    return matching;
}

vec4 ColliderObject::createCenter(const mat4 &tm, const mat4 &sm, ColliderObject *col) {
    return tm * sm * col->originalCenter();
}

vec4 ColliderObject::createOffset(const mat4 &tm, const mat4 &sm, const vec4 &center, ColliderObject *col) {
    vec4 minOffset = tm * sm * col->minPoints();
    return center - minOffset;
}
//float s1, vec3 oc1, vec3 mp1,
int ColliderObject::getCollisionRaw(vec3 p1, ColliderObject *c1, vec3 p2, ColliderObject *c2) {
    // Center = critical section?
    int matching = 0;  // Number of axis that have collided
    if (c1 == nullptr || c2 == nullptr) {
        cerr << "Error: Cannot get collision for NULL GameObjects!\n";
        return NO_MATCH;
    }
    auto tm1 = glm::translate(mat4(1.0f), p1);
    auto sm1 = c1->pScaleMatrix();
    auto center1 = createCenter(tm1, sm1, c1);
    auto offset1 = createOffset(tm1, sm1, center1, c1);

    auto tm2 = glm::translate(mat4(1.0f), p2);
    auto sm2 = c2->pScaleMatrix();
    auto center2 = createCenter(tm2, sm2, c2);
    auto offset2 = createOffset(tm2, sm2, center2, c2);

    auto delta = center1 - center2;
    auto range = offset1 + offset2;

    for (uint i = 0; i < 3; ++i) {
        float res = range[i] - abs(delta[i]);
        if (range[i] > abs(delta[i]) && abs(res) > EPSILON) {
            matching |= (1<<i);
        }
    }
    return matching;
}

void ColliderObject::update() {
    // Easy wireframe rendering is unsupported in OpenGL ES
#ifndef GFX_EMBEDDED
    if (drawCollider_) render();
#endif  // GFX_EMBEDDED
}

void ColliderObject::render() {
    if (poly_.get()->numberOfObjects > 0) {
        gfxController_->setProgram(programId_);
        gfxController_->polygonRenderMode(RenderMode::LINE);
        gfxController_->setCapability(GfxCapability::CULL_FACE, false);
        mat4 MVP = pVpMatrix_ * pTranslateMatrix_ * pScaleMatrix_;
        gfxController_->sendFloatMatrix(mvpId_, 1, glm::value_ptr(MVP));
        // HINT: Render loops should really just be (bind Vao, draw triangles)
        gfxController_->bindVao(vao_);
        gfxController_->drawTriangles(poly_.get()->pointCount[0]);
    }
}

void ColliderObject::createCollider() {
    // Initialize VAO
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);
    cout << "Building collider for " << objectName_ << endl;
    float min[3] = {999, 999, 999}, tempMin[3] = {999, 999, 999};
    float max[3] = {-999, -999, -999}, tempMax[3] = {-999, -999, -999};
    // Set MVP ID for collider object
    mvpId_ = gfxController_->getShaderVariable(programId_, "MVP").get();
    // Go through objects and get absolute min/max points
    for (auto vertex : target_->vertices) {
        for (int i = 0; i < 3; i++) {
            // Calculate min
            tempMin[i] = getColliderVertices(vertex, i, [](float a, float b) { return a < b; });
            // Calculate max
            tempMax[i] = getColliderVertices(vertex, i, [](float a, float b) { return a > b; });
            if (tempMin[i] < min[i]) {
                min[i] = tempMin[i];
            }
            if (tempMax[i] > max[i]) {
                max[i] = tempMax[i];
            }
        }
    }
    // Manually build triangles for cube collider
    vector<float> colliderVertices = {
        // First face
        min[0], min[1], min[2],
        min[0], min[1], max[2],
        min[0], max[1], min[2],
        min[0], max[1], max[2],
        min[0], max[1], min[2],
        min[0], min[1], max[2],
        // Second face
        min[0], min[1], max[2],
        min[0], max[1], max[2],
        max[0], min[1], max[2],
        min[0], max[1], max[2],
        max[0], min[1], max[2],
        max[0], max[1], max[2],
        // Third face
        max[0], max[1], max[2],
        max[0], min[1], max[2],
        max[0], max[1], min[2],
        max[0], min[1], min[2],
        max[0], min[1], max[2],
        max[0], max[1], min[2],
        // Fourth face
        max[0], max[1], min[2],
        min[0], max[1], min[2],
        max[0], min[1], min[2],
        min[0], min[1], min[2],
        min[0], max[1], min[2],
        max[0], min[1], min[2],
        // Fifth face
        min[0], max[1], min[2],
        max[0], max[1], min[2],
        min[0], max[1], max[2],
        max[0], max[1], max[2],
        max[0], max[1], min[2],
        min[0], max[1], max[2],
        // Sixth Face
        min[0], min[1], min[2],
        max[0], min[1], min[2],
        min[0], min[1], max[2],
        max[0], min[1], max[2],
        max[0], min[1], min[2],
        min[0], min[1], max[2]
    };
    auto pointCount = colliderVertices.size() / 3;
    poly_ = std::make_shared<Polygon>(pointCount, colliderVertices);
    gfxController_->generateBuffer(&poly_.get()->shapeBufferId[0]);
    gfxController_->bindBuffer(poly_.get()->shapeBufferId[0]);
    gfxController_->sendBufferData(sizeof(float) * colliderVertices.size(), &colliderVertices[0]);
    gfxController_->enableVertexAttArray(0, 3, sizeof(float), 0);
    // Set the correct center points
    for (int i = 0; i < 3; i++) {
        center_[i] = max[i] - ((abs(max[i] - min[i])) / 2);
    }
    center_[3] = 1;  // SET W!!!
    originalCenter_ = center_;
    // Update the offset for the collider to be distance between center and edge
    for (int i = 0; i < 3; i++) {
        minPoints_[i] = min[i];
    }
    minPoints_[3] = 1;  // SET W!!!
    gfxController_->bindVao(0);
}

ColliderObject::~ColliderObject() {
}

float ColliderObject::getColliderVertices(vector<float> vertices, int axis,
    bool (*test)(float a, float b)) {
    if (vertices.size() < 3) {
        cerr << "Error: Vertices vector is empty!\n";
        return 0.0f;
    }
    float currentMin = vertices[axis];
    for (uint i = 0; i < vertices.size() / 3; i++) {
        float tempMin = vertices[i * 3 + axis];
        if (test(tempMin, currentMin)) {
            currentMin = tempMin;
        }
    }
    return currentMin;
}

vec3 ColliderObject::getEdgePoint(ColliderObject *object, vec3 epSign) {
    assert(object != nullptr);  // Eventually handle this gracefully, I just need it to explode for now
    // Iterate through each axis
    vec3 result(0);
    vec3 deltaBase = center_ - object->center();
    vec3 delta = abs(deltaBase);
    vec3 range = offset_ + object->offset();
    vec3 edgePoint = (range - delta);
    for (int i = 0; i < 3; ++i) {
        // Do this more efficiently
        if ((epSign[i] > 0.0f && deltaBase[i] < 0.0f) ||
            (epSign[i] < 0.0f && deltaBase[i] > 0.0f)) {
            /**
             * This is a special case where a single update results in our current object moving PASSED
             * the center of the other object we're testing collision on. This creates a contradition
             * between delta and epSign's sign. For proper behavior, we will add the range (both offsets)
             * to the edge point for this axis. Realistically, this shouldn't really manifest itself as a
             * glitch in production, but it ensures that we pop out of the other object properly. Otherwise
             * we may see excessive collisions before the object is properly corrected. This should also
             * reduce jerkiness of object's colliding in this case.
             */
            edgePoint[i] += range[i];
        }
    }
    result = edgePoint;

    static int collCount;
    printf("--- Collision %d ---\n\n", collCount / 2 + 1);

    // Let's print out relevant information neatly
    printf("* Center (%f, %f, %f)\n", center_.x, center_.y, center_.z);
    printf("* Other Center (%f, %f, %f)\n", object->center().x, object->center().y, object->center().z);
    printf("* Center Distance (%f, %f, %f)\n", fabs(object->center().x - center_.x), fabs(object->center().y - center_.y), fabs(object->center().z - center_.z));
    printf("* Projection (%f, %f, %f)\n", center_.x + result.x, center_.y + result.y, center_.z + result.z);
    printf("* Edge Point is (%f, %f, %f)\n", edgePoint.x, edgePoint.y, edgePoint.z);
    printf("* Result (%f, %f, %f)\n", result.x, result.y, result.z);
    printf("* Offset (%f, %f, %f)\n", offset_.x, offset_.y, offset_.z);
    printf("* Other Offset (%f, %f, %f)\n", object->offset().x, object->offset().y, object->offset().z);

    collCount++;
    return result; // Return half - the idea is that the other object will get the other half of this value...
}

vec3 ColliderObject::getEdgePointPosInf(ColliderObject *object) {
    assert(object != nullptr);  // Eventually handle this gracefully, I just need it to explode for now
    // Iterate through each axis
    vec3 result(0);
    vec3 deltaBase = object->center() - center_;
    vec3 delta = abs(deltaBase);
    vec3 range = offset_ + object->offset();
    vec3 edgePoint = (range - delta);

    vec3 x1_delta = center_ - object->center();

    float highestDistance = 0.0f;
    //int distindex = 0;
    for (int i = 0; i < 3; ++i) {
        auto absDist = fabs(x1_delta[i]);
        if (absDist > highestDistance) {
            //distindex = i;
            highestDistance = absDist;
        }
    }
    assert(highestDistance != 0.0f);
    vec3 normalizedDistance = x1_delta / vec3(highestDistance);
    result = edgePoint * normalizedDistance;
    return result; // Return half - the idea is that the other object will get the other half of this value...
}
