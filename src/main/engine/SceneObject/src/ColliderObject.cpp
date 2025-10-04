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
ColliderObject::ColliderObject(std::shared_ptr<Polygon> target, unsigned int programId, mat4 *translateMatrix,
    mat4 *scaleMatrix, mat4 *vpMatrix, ObjectType type, string objectName, GfxController *gfxController) :
    SceneObject(type, objectName, gfxController), target_ { target }, pTranslateMatrix_ { translateMatrix },
    pScaleMatrix_ { scaleMatrix }, pVpMatrix_ { vpMatrix } {
    programId_ = programId;
    createCollider();
}

/**
 * @brief Constructor for 2D collider objects.
 */
ColliderObject::ColliderObject(const vector<float> &vertTexData, unsigned int programId, mat4 *translateMatrix,
    mat4 *scaleMatrix, mat4 *vpMatrix, ObjectType type, string objectName, GfxController *gfxController) :
    SceneObject(type, objectName, gfxController), pTranslateMatrix_ { translateMatrix }, pScaleMatrix_ { scaleMatrix },
    pVpMatrix_ { vpMatrix } {
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
    center_ = (*pTranslateMatrix_) * (*pScaleMatrix_) * originalCenter_;
    vec4 minOffset = (*pTranslateMatrix_) * (*pScaleMatrix_) * minPoints_;
    // Use rescaled edge points to calculate offset on the fly!
    for (int i = 0; i < 4; i++) {
        offset_[i] = center_[i] - minOffset[i];
    }
}

/**
 * @brief Checks if this collider is colliding or about to collide with another collider
 *
 * @param object other collider to check collision with
 * @param moving the current object's translation
 * @return int -1 if error, 0 for no collision, 1 for colliding, 2 for about to collide
 */
int ColliderObject::getCollision(ColliderObject *object, vec3 moving) {
    // Center = critical section?
    int matching = 0;  // Number of axis that have collided
    if (object == nullptr) {
        cerr << "Error: Cannot get collision for NULL GameObjects!\n";
        return -1;
    }

    // First check if the two objects are currently colliding
    for (int i = 0; i < 3; i++) {
        float delta = abs(object->center()[i] - this->center()[i]);
        float range = this->offset()[i] + object->offset()[i];
        if (range >= delta) {
            matching++;
        }
    }
    // Return if the objects are currently colliding
    if (matching == 3) return 1;
    matching = 0;
    for (int i = 0; i < 3; i++) {
        float delta = abs(object->center()[i] - this->center()[i] + moving[i]);
        float range = this->offset()[i] + object->offset()[i];
        if (range >= delta) {
            matching++;
        }
    }
    // Return if the objects are about to collide
    if (matching == 3) return 2;
    return 0;
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
        mat4 MVP = (*pVpMatrix_) * (*pTranslateMatrix_) * (*pScaleMatrix_);
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
    cout << "Building collider for " << objectName << endl;
    float min[3] = {999, 999, 999}, tempMin[3] = {999, 999, 999};
    float max[3] = {-999, -999, -999}, tempMax[3] = {-999, -999, -999};
    // Set MVP ID for collider object
    mvpId_ = gfxController_->getShaderVariable(programId_, "MVP").get();
    // Go through objects and get absolute min/max points
    for (auto it = target_->vertices.begin(); it != target_->vertices.end(); ++it) {
        for (int i = 0; i < 3; i++) {
            // Calculate min
            tempMin[i] = getColliderVertices((*it), i, [](float a, float b) { return a < b; });
            // Calculate max
            tempMax[i] = getColliderVertices((*it), i, [](float a, float b) { return a > b; });
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

vec3 ColliderObject::getEdgePoint(ColliderObject *object, vec3 velocity) {
    assert(object != nullptr);  // Eventually handle this gracefully, I just need it to explode for now
    // Iterate through each axis
    vec3 result(0);
    for (int i = 0; i < 3; ++i) {
        // Double check the offset is just the distance between center point and edge and not edge to edge
        float delta = abs(object->center()[i] - center_[i]);
        float range = offset_[i] + object->offset()[i];
        // For this axis, how much is this object inside of the other?
        float edgePoint = range - delta;
        // Now the edge point contains how much we're inside of the other object for this axis
        // Do not add edgePoint if the distance is lte 0
        if (edgePoint <= 0) continue;
        // Now determine direction based on the velocity - this is the direction the object WAS traveling...
        result[i] = velocity[i] > 0.0f ? -edgePoint : edgePoint;
    }
    printf("This center point %f, %f, %f\n", center_.x, center_.y, center_.z);
    printf("Other center point is %f, %f, %f\n", object->center().x, object->center().y, object->center().z);
    printf("This offset %f, %f, %f\n", offset_.x, offset_.y, offset_.z);
    printf("Other offset %f, %f, %f\n", object->offset().x, object->offset().y, object->offset().z);
    printf("velocity is %f, %f, %f\n", velocity.x, velocity.y, velocity.z);
    printf("Edge Point is %f, %f, %f\n", result.x, result.y, result.z);
    return result;
}
