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

GameObject::GameObject(gameObjectInfo objectInfo):
    SceneObject(objectInfo.position, objectInfo.rotation, objectInfo.objectName, objectInfo.scale,
    objectInfo.characterModel->programId), model { objectInfo.characterModel }, cameraId { objectInfo.camera } {
    collider.collider = nullptr;  // Default to not having a collider
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
    translateMatrix = glm::translate(mat4(1.0f), objectInfo.position);
    rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
            vec3(0, 0, 1));
    // Grab IDs for shared variables between app and program (shader)
    rotateId = glGetUniformLocation(programId, "rotate");
    scaleId = glGetUniformLocation(programId, "scale");
    translateId = glGetUniformLocation(programId, "move");
    vpId = glGetUniformLocation(programId, "VP");
    hasTextureId = glGetUniformLocation(programId, "hasTexture");
    directionalLightId = glGetUniformLocation(programId, "directionalLight");
    luminanceId = glGetUniformLocation(programId, "luminance");
    rollOffId = glGetUniformLocation(programId, "rollOff");
    mvpId = -1;
    vpMatrix = mat4(1.0f);  // Default VP matrix to identity matrix
}

GameObject::~GameObject() {
    /// @todo: Run cleanup methods here
    cout << "Destroying gameobject" << objectName << endl;
    deleteTextures();
}

colliderInfo GameObject::getCollider(void) {
    // Update center position with model matrix then return
    collider.center = translateMatrix * scaleMatrix * rotateMatrix * collider.originalCenter;
    vec4 minOffset = translateMatrix * scaleMatrix * rotateMatrix * collider.minPoints;
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
    // Draw each shape individually
    for (int i = 0; i < model->numberOfObjects; i++) {
        glUseProgram(programId);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // Update our model transformation matrices
        translateMatrix = glm::translate(mat4(1.0f), position);
        rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
                vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
                vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
                vec3(0, 0, 1));
        scaleMatrix = glm::scale(vec3(scale, scale, scale));
        // Send our shared variables over to our program (shader)
        glUniform1f(luminanceId, luminance);
        glUniform1f(rollOffId, rollOff);
        glUniform3fv(directionalLightId, 1, &directionalLight[0]);
        glUniformMatrix4fv(vpId, 1, GL_FALSE, &vpMatrix[0][0]);
        glUniformMatrix4fv(translateId, 1, GL_FALSE, &translateMatrix[0][0]);
        glUniformMatrix4fv(scaleId, 1, GL_FALSE, &scaleMatrix[0][0]);
        glUniformMatrix4fv(rotateId, 1, GL_FALSE, &rotateMatrix[0][0]);
        glUniform1i(hasTextureId, hasTexture[i]);
        if (hasTexture[i]) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model->textureId[i]);
            glUniform1i(model->textureUniformId, 0);
        }
        // Actually start drawing polygons :)
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, model->shapeBufferId[i]);
        glVertexAttribPointer(
                              0,            // attribute 0. No particular reason for 0, but must match
                                            // the layout in the shader.
                              3,            // size
                              GL_FLOAT,     // type
                              GL_FALSE,     // normalized?
                              0,            // stride
                              0);           // array buffer offset
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, model->normalBufferId[i]);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        if (hasTexture[i]) {
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, model->textureCoordsId[i]);
            glVertexAttribPointer(
                                  1,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  0,
                                  0);
            glDrawArrays(GL_TRIANGLES, 0, model->pointCount[i] * 3);
            glDisableVertexAttribArray(1);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, model->pointCount[i] * 3);
        }
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(0);
    }
    if (collider.collider != NULL) {
        // After drawing the gameobject, draw the collider
        glUseProgram(collider.collider->programId);  // grab the programId from the object
        glDisable(GL_CULL_FACE);  // Just do it
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mat4 MVP = vpMatrix * translateMatrix * scaleMatrix * rotateMatrix;
        glUniformMatrix4fv(mvpId, 1, GL_FALSE, &MVP[0][0]);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, collider.collider->shapeBufferId[0]);
        glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match
                                                  // the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              0);            // array buffer offset
        glDrawArrays(GL_TRIANGLES, 0, collider.collider->pointCount[0] * 3);
        glDisableVertexAttribArray(0);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void GameObject::deleteTextures() {
    for (int i = 0; model->numberOfObjects; i++) {
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
    if (model == NULL) {
        cerr <<
            "Error: Cannot create collider because GameObject model is missing"
            << endl;
        return -1;
    }
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
    collider.collider = new Polygon(pointCount, shaderId, colliderVertices);
    glGenBuffers(1, &(collider.collider->shapeBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, collider.collider->shapeBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * pointCount,
        &(collider.collider->vertices[0][0]), GL_STATIC_DRAW);
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
