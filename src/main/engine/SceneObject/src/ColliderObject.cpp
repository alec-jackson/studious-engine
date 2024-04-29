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
ColliderObject::ColliderObject(Polygon *target, GLuint programId, const mat4 &translateMatrix, const mat4 &scaleMatrix,
    const mat4 &vpMatrix, ObjectType type, string objectName, GfxController *gfxController) :
    SceneObject(type, objectName, gfxController), target_ { target }, translateMatrix_ { translateMatrix },
    scaleMatrix_ { scaleMatrix }, vpMatrix_ { vpMatrix } {
    createCollider(programId);
}

void ColliderObject::updateCollider() {
    // Update center position with model matrix
    center_ = translateMatrix_ * scaleMatrix_ * originalCenter_;
    vec4 minOffset = translateMatrix_ * scaleMatrix_ * minPoints_;
    // Use rescaled edge points to calculate offset on the fly!
    for (int i = 0; i < 4; i++) {
        offset_[i] = center_[i] - minOffset[i];
    }
}

int ColliderObject::getCollision(ColliderObject *object, vec3 moving) {
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
    render();
#endif
}

void ColliderObject::render() {
    if (poly_->numberOfObjects > 0) {
        gfxController_->setProgram(poly_->programId);
        gfxController_->polygonRenderMode(RenderMode::LINE);
        gfxController_->setCapability(GfxCapability::CULL_FACE, false);
        mat4 MVP = vpMatrix_ * translateMatrix_ * scaleMatrix_;
        gfxController_->sendFloatMatrix(mvpId_, 1, glm::value_ptr(MVP));
        // Don't need to bind vao -> Should happen in render loop
        gfxController_->bindVao(vao_);
        // Enable vertex data
        gfxController_->bindBuffer(poly_->shapeBufferId[0]);
        gfxController_->enableVertexAttArray(0, 3);
        gfxController_->drawTriangles(poly_->pointCount[0]);
        gfxController_->disableVertexAttArray(0);
    }
}

void ColliderObject::createCollider(GLuint programId) {
    // Initialize VAO
    gfxController_->initVao(&vao_);
    cout << "Building collider for " << objectName << endl;
    GLfloat min[3] = {999, 999, 999}, tempMin[3] = {999, 999, 999};
    GLfloat max[3] = {-999, -999, -999}, tempMax[3] = {-999, -999, -999};
    // Set MVP ID for collider object
    mvpId_ = gfxController_->getShaderVariable(programId, "MVP").get();
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
    // Save resources on embedded systems
#ifndef GFX_EMBEDDED
    // Manually build triangles for cube collider
    vector<GLfloat> colliderVertices = {
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
    poly_ = new Polygon(pointCount, programId, colliderVertices);
    gfxController_->generateBuffer(&poly_->shapeBufferId[0]);
    gfxController_->bindBuffer(poly_->shapeBufferId[0]);
    gfxController_->sendBufferData(sizeof(GLfloat) * colliderVertices.size(), &colliderVertices[0]);
#endif
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
}

ColliderObject::~ColliderObject() {
    delete poly_;
}

GLfloat ColliderObject::getColliderVertices(vector<GLfloat> vertices, int axis,
    bool (*test)(float a, float b)) {
    if (vertices.size() < 3) {
        cerr << "Error: Vertices vector is empty!\n";
        return 0.0f;
    }
    GLfloat currentMin = vertices[axis];
    for (int i = 0; i < vertices.size() / 3; i++) {
        GLfloat tempMin = vertices[i * 3 + axis];
        if (test(tempMin, currentMin)) {
            currentMin = tempMin;
        }
    }
    return currentMin;
}

