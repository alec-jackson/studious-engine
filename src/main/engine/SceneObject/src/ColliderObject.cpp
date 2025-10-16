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
ColliderObject::ColliderObject(std::shared_ptr<Polygon> target, uint programId, SceneObject *owner) :
    SceneObject(owner->type(), owner->objectName() + "-Collider", owner->gfxController()), target_ { target }, pTranslateMatrix_ { owner->translateMatrix() },
    pScaleMatrix_ { owner->scaleMatrix() }, pVpMatrix_ { owner->vpMatrix() } {
    programId_ = programId;
    createCollider();
}

/**
 * @brief Constructor for 2D collider objects.
 */
ColliderObject::ColliderObject(const vector<float> &vertTexData, unsigned int programId, SceneObject *owner) :
    SceneObject(owner->type(), owner->objectName() + "-Collider", owner->gfxController()), pTranslateMatrix_ { owner->translateMatrix() }, pScaleMatrix_ { owner->scaleMatrix() },
    pVpMatrix_ { owner->vpMatrix() } {
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
        if (range > delta) {
            matching++;
        }
    }
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
    vec3 deltaBase = object->center() - center_;
    vec3 delta = abs(deltaBase);
    vec3 range = offset_ + object->offset();
    vec3 edgePoint = range - delta;
    edgePoint *= EDGE_POINT_PERCENT_SCALAR;
    /* There is a problem with the current algorithm :
     *
     * Considering the direction of the velocity. When an object bounces off of another,
     * both objects launch off in their own direction. The velocity itself will describe the angle...
     *
     * Rather than finding the edge point by just moving the object to the collider's boundary,
     * let's consider the following algorithm:
     *
     * velocity = <v1, v2, v3>
     *
     * What is the highest velocity in the vector (abs val)? If it's V1, then normalize the other
     * velocities by this value in a new vector (normalized velocity):
     *
     * // First filter out velocities of zero to avoid div by zero errors
     *
     * normalizedVelocity = <v1/abs(v1), v1/abs(v1), v1/abs(v1)>
     *
     * Then multiply this by the edge point. The zeroes will not excessively move
     * the object, while the direction of the bounce, and the amount of the bounce is maintained.
     *
     *
     */
    // float highestVelocity = 0.0f;
    // for (int i = 0; i < 3; ++i) {
    //     auto absVel = fabs(velocity[i]);
    //     if (absVel > highestVelocity) highestVelocity = absVel;
    // }
    // assert(highestVelocity != 0.0f);
    // vec3 normalizedVelocity = velocity / vec3(highestVelocity);
    // result = edgePoint * normalizedVelocity;

    /**
     * The velocity direction solution has a huge problem: What if the resulting collision sees both objects going
     * in the same direction? This will effectively give both objects the SAME edge point values, causing them to clip
     * into each other ad infinitum.
     *
     * What are the potential solutions here?
     *
     * When calculating the resulting direction, we can just check "is this object on the left or right (pos or neg) side
     * of the other?". So base the resulting edge point's direction for each axis based on whether or not we are on
     * the right or left side. We still need some kind of scalar to dampen each axis edge point.
     *
     * POTENTIAL SOLUTION: Say you have two points:
     *
     * |
     * |    x2 (4, 4)
     * |
     * |   x1 (3, 2)
     * |
     * _________________________
     *
     * Where x1 has an offset of [3, 3], and x2 has an offset of [5, 5]. How do we calculate the edge point? We would
     * want something like the following:
     *
     * First calculate distance on that axis, then find direction. So... Distance + direction = delta :)
     *
     * x1_delta = x1 - x2 = (3, 2) - (4, 4) = (-1, -2)
     *
     * x2_delta = x2 - x1 = (4, 4) - (3, 2) = (1, 2)
     *
     * This alone isn't sufficient. If we applied this transformation to x1 and x2, we would get:
     *
     * x1 = (3, 2) + (-1, -2) = (2, 0)
     * x2 = (4, 4) + (1,   2) = (5, 6)
     *
     * When we do a collision check afterwards, we see
     *
     * range = fabs(x1 - x2) = (2, 0) - (5, 6) = fabs((-3, -6)) = (3, 6)
     *
     * offset sum = offset_x1 + offset_x2 = (3, 3) + (5, 5) = (8, 8)
     *
     * range (3, 6) is less than offset sum (8, 8), therefore the objects are still colliding. These values
     * need to be boosted using the offset values. These values just tell us the actual direction to bounce.
     *
     * for each object do:
     *
     * Find the axis with the highest distance:
     * highestDistance = max(delta.x, y, z)
     * Normalize similar to the velocity method:
     *
     * normalizedDistance = <delta.x/highestDistance, delta.y/highestDistance, delta.z/highestDistance>
     *
     * This will maintain the direction of the "bounce back" we get from the collision.
     *
     * Now we can apply this to the initial edge point vector - the distance we NEED to travel
     * to get out of collision bounds!
     */

    vec3 x1_delta = center_ - object->center();

    float highestDistance = 0.0f;
    for (int i = 0; i < 3; ++i) {
        auto absDist = fabs(x1_delta[i]);
        if (absDist > highestDistance) highestDistance = absDist;
    }
    assert(highestDistance != 0.0f); // Remove??
    vec3 normalizedDistance = x1_delta / vec3(highestDistance);
    result = edgePoint * normalizedDistance;

    static int collCount;
    printf("--- Collision %d ---\n\n", collCount / 2 + 1);

    // Let's print out relevant information neatly
    printf("* Center (%f, %f, %f)\n", center_.x, center_.y, center_.z);
    printf("* Other Center (%f, %f, %f)\n", object->center().x, object->center().y, object->center().z);
    printf("* Center Distance (%f, %f, %f)\n", fabs(object->center().x - center_.x), fabs(object->center().y - center_.y), fabs(object->center().z - center_.z));
    printf("* Projection (%f, %f, %f)\n", center_.x + result.x, center_.y + result.y, center_.z + result.z);
    printf("* Edge Point is (%f, %f, %f)\n", result.x, result.y, result.z);
    printf("* Final Velocity (%f, %f, %f)\n", velocity.x, velocity.y, velocity.z);
    printf("* Offset (%f, %f, %f)\n", offset_.x, offset_.y, offset_.z);
    printf("* Other Offset (%f, %f, %f)\n", object->offset().x, object->offset().y, object->offset().z);

    collCount++;
    /*
    printf("This center point %f, %f, %f\n", center_.x, center_.y, center_.z);
    printf("Other center point is %f, %f, %f\n", object->center().x, object->center().y, object->center().z);
    printf("This offset %f, %f, %f\n", offset_.x, offset_.y, offset_.z);
    printf("Other offset %f, %f, %f\n", object->offset().x, object->offset().y, object->offset().z);
    printf("velocity is %f, %f, %f\n", velocity.x, velocity.y, velocity.z);
    printf("Edge Point is %f, %f, %f\n", result.x, result.y, result.z);
    */
    return result; // Return half - the idea is that the other object will get the other half of this value...
}
