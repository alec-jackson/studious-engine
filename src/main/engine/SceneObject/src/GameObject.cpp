/**
 * @file GameObject.cpp
 * @author Christian Galvez
 * @brief Implementation for GameObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <GameObject.hpp>

GameObject::GameObject(Polygon *characterModel, vec3 position, vec3 rotation, GLfloat scale, int camera,
    string objectName, GfxController *gfxController):
    SceneObject(position, rotation, objectName, scale, characterModel->programId, gfxController),
    model { characterModel }, cameraId { camera } {
    luminance = 1.0f;
    rollOff = 0.9f;  // Rolloff describes the intensity of the light dropoff
    directionalLight = vec3(0, 0, 0);
    viewMode = PERSPECTIVE;
    // Populate the hasTexture vector with texture info
    for (int i = 0; i < model->numberOfObjects; i++) {
        if (model->textureCoordsId[i] == UINT_MAX) {
            hasTexture.push_back(0);  // No texture found for obj i
        } else {
            hasTexture.push_back(1);  // Texture found for obj i
        }
    }
    scaleMatrix = glm::scale(vec3(scale, scale, scale));
    translateMatrix = glm::translate(mat4(1.0f), position);
    rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
            vec3(0, 0, 1));
    // Grab IDs for shared variables between app and program (shader)
    modelId = gfxController_->getShaderVariable(programId, "model").get();
    vpId = gfxController_->getShaderVariable(programId, "VP").get();
    hasTextureId = gfxController_->getShaderVariable(programId, "hasTexture").get();
    directionalLightId = gfxController_->getShaderVariable(programId, "directionalLight").get();
    luminanceId = gfxController_->getShaderVariable(programId, "luminance").get();
    rollOffId = gfxController_->getShaderVariable(programId, "rollOff").get();
    mvpId = -1;
    vpMatrix = mat4(1.0f);  // Default VP matrix to identity matrix
}

GameObject::~GameObject() {
    /// @todo: Run cleanup methods here
    cout << "Destroying gameobject" << objectName << endl;
    deleteTextures();
}

colliderInfo &GameObject::getCollider(void) {
    // Update center position with model matrix then return
    collider.center = translateMatrix * scaleMatrix * collider.originalCenter;
    vec4 minOffset = translateMatrix * scaleMatrix * collider.minPoints;
    // Use rescaled edge points to calculate offset on the fly!
    for (int i = 0; i < 4; i++) {
        collider.offset[i] = collider.center[i] - minOffset[i];
    }
    return collider;
}

/// @todo Update and doxygenize this method
/* 
 (void) drawShape draws the current GameObject inside of the scene. This is the
 method that interacts with OpenGL the most (just to let you know). Method
 walks through all of the objects inside of the current model and renders them
 one by one in the for loop below.

 (void) drawShape does not return any values.
*/
void GameObject::render() {
    // Send GameObject to render method
    // Draw each shape individually
    for (int i = 0; i < model->numberOfObjects; i++) {
        gfxController_->setProgram(programId);
        gfxController_->polygonRenderMode(RenderMode::FILL);
        // Update our model transformation matrices
        translateMatrix = glm::translate(mat4(1.0f), position);
        rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
                vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
                vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
                vec3(0, 0, 1));

        scaleMatrix = glm::scale(vec3(scale, scale, scale));
        auto modelMatrix = translateMatrix * rotateMatrix * scaleMatrix;
        // Send our shared variables over to our program (shader)
        gfxController_->sendFloat(luminanceId, luminance);
        gfxController_->sendFloat(rollOffId, rollOff);
        gfxController_->sendFloatVector(directionalLightId, 1, glm::value_ptr(directionalLight));
        gfxController_->sendFloatMatrix(vpId, 1, glm::value_ptr(vpMatrix));
        gfxController_->sendFloatMatrix(modelId, 1, glm::value_ptr(modelMatrix));
        gfxController_->sendInteger(hasTextureId, hasTexture[i]);
        if (hasTexture[i]) {
            // Bind texture to sampler for polygon rendering below
            gfxController_->bindTexture(model->textureId[i], model->textureUniformId);
        }
        // Actually start drawing polygons :)
        gfxController_->render(model->shapeBufferId[i], model->textureCoordsId[i], model->normalBufferId[i],
        model->pointCount[i] * 3);
    }
    /// @todo Move collider code to a separate collider SceneObject
    if (collider.collider.numberOfObjects > 0) {
        // After drawing the gameobject, draw the collider
        glUseProgram(collider.collider.programId);  // grab the programId from the object
        glDisable(GL_CULL_FACE);  // Just do it
        gfxController_->polygonRenderMode(RenderMode::LINE);
        mat4 MVP = vpMatrix * translateMatrix * scaleMatrix;
        gfxController_->sendFloatMatrix(mvpId, 1, glm::value_ptr(MVP));
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, collider.collider.shapeBufferId[0]);
        glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match
                                                  // the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              0);            // array buffer offset
        glDrawArrays(GL_TRIANGLES, 0, collider.collider.pointCount[0] * 3);
        glDisableVertexAttribArray(0);
    }
}

void GameObject::deleteTextures() {
    cout << "GameObject::deleteTextures" << endl;
    for (int i = 0; i < model->numberOfObjects; i++) {
        if (hasTexture[i]) {
            glDeleteTextures(1, &textureId);
            hasTexture[i] = false;
        }
    }
}

GLfloat GameObject::getColliderVertices(vector<GLfloat> vertices, int axis,
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

int GameObject::createCollider(int shaderId) {
    cout << "Building collider for " << objectName << endl;
    GLfloat min[3] = {999, 999, 999}, tempMin[3] = {999, 999, 999};
    GLfloat max[3] = {-999, -999, -999}, tempMax[3] = {-999, -999, -999};
    // Set MVP ID for collider object
    mvpId = glGetUniformLocation(shaderId, "MVP");
    vector<vector<GLfloat>>::iterator it;
    // Go through objects and get absolute min/max points
    for (it = model->vertices.begin(); it != model->vertices.end(); ++it) {
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
    auto pointCount = colliderVertices.size();
    collider.collider = Polygon(pointCount, shaderId, colliderVertices);
    glGenBuffers(1, &(collider.collider.shapeBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, collider.collider.shapeBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * pointCount,
        &(collider.collider.vertices[0][0]), GL_STATIC_DRAW);
    // Set the correct center points
    for (int i = 0; i < 3; i++) {
        collider.center[i] = max[i] - ((abs(max[i] - min[i])) / 2);
    }
    collider.center[3] = 1;  // SET W!!!
    collider.originalCenter = collider.center;
    // Update the offset for the collider to be distance between center and edge
    for (int i = 0; i < 3; i++) {
        collider.minPoints[i] = min[i];
    }
    collider.minPoints[3] = 1;  // SET W!!!
    return 0;
}