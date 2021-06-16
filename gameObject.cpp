#include "modelImport.hpp"
#include "gameObject.hpp"

void GameObject::configureGameObject(gameObjectInfo objectInfo) {
    //Arbitrary directional light value (PREVENT SEGFAULT)
    programID = objectInfo.characterModel->programID;
    collider = objectInfo.colliderObject;
    luminance = 1.0f;
    rollOff = 0.9f;
    directionalLight = vec3(0,0,0);
    model = objectInfo.characterModel;
    configured = true;
    currentCamera = objectInfo.camera;
    // Populate the hasTexture vector with texture info
    for (int i = 0; i < model->numberOfObjects; i++) {
        if (model->textureCoordsID[i] == UINT_MAX) {
            hasTexture.push_back(0); // No texture found for obj i
        } else {
            hasTexture.push_back(1); // Texture found for obj i
        }
    }
    // Save initial scale, rot, pos for object
    scale = objectInfo.scaleVec[0];
    rot = vec3(objectInfo.rotAngle);
    pos = objectInfo.pos;
    scaleMatrix = glm::scale(objectInfo.scaleVec);
    translateMatrix = glm::translate(mat4(1.0f), objectInfo.pos);
    rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(objectInfo.rotAngle),
        objectInfo.rotateAxis);
    collisionTag = objectInfo.collisionTagName;
    rotateID = glGetUniformLocation(programID, "rotate");
    scaleID = glGetUniformLocation(programID, "scale");
    translateID = glGetUniformLocation(programID, "move");
    vpID = glGetUniformLocation(programID, "VP");
    hasTextureID = glGetUniformLocation(programID, "hasTexture");
    directionalLightID = glGetUniformLocation(programID, "directionalLight");
    luminanceID = glGetUniformLocation(programID, "luminance");
    rollOffID = glGetUniformLocation(programID, "rollOff");
    if (collider != NULL) {
        MVPID = glGetUniformLocation(collider->programID, "MVP");
    }
    vpMatrix = mat4(1.0f); // Set the VP matrix to the identity matrix by default.
}

string GameObject::getCollider(void) {
    if (collisionTag.empty()) {
        cerr << "GameObject was not assigned a collider tag!\n";
    }
    return collisionTag;
}
// rotateObject takes a vec3 containing x, y and z rotation in degrees.
void GameObject::rotateObject(vec3 rotation) {
    rot = rotation;
}
void GameObject::sendPosition(vec3 position) {
    pos = position;
}
void GameObject::sendScale(GLfloat uniformScale) {
    scale = uniformScale;
}
GLfloat GameObject::getScale() {
    return scale;
}
void GameObject::setDirectionalLight(vec3 newLight) {
    directionalLight = newLight;
}
void GameObject::drawShape() {
    infoLock.lock(); // Obtain lock for all current object data
    // Draw each shape individually
    if (!configured) {
        cerr << "GameObject with tag " << collisionTag
            << " has not been configured yet!\n";
        infoLock.unlock();
        return;
    }
    for (int i = 0; i < model->numberOfObjects; i++) {
        glUseProgram(programID);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        translateMatrix = glm::translate(mat4(1.0f), pos);
        rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(rot[0]),
                vec3(1,0,0))  *glm::rotate(mat4(1.0f), glm::radians(rot[1]),
                vec3(0,1,0))  *glm::rotate(mat4(1.0f), glm::radians(rot[2]),
                vec3(0,0,1)); // Refactor
        scaleMatrix = glm::scale(vec3(scale, scale, scale));
        glUniform1f(luminanceID, luminance);
        glUniform1f(rollOffID, rollOff);
        glUniform3fv(directionalLightID, 1, &directionalLight[0]);
        glUniformMatrix4fv(vpID, 1, GL_FALSE, &vpMatrix[0][0]);
        glUniformMatrix4fv(translateID, 1, GL_FALSE, &translateMatrix[0][0]);
        glUniformMatrix4fv(scaleID, 1, GL_FALSE, &scaleMatrix[0][0]);
        glUniformMatrix4fv(rotateID, 1, GL_FALSE, &rotateMatrix[0][0]);
        glUniform1i(hasTextureID, hasTexture[i]);
        if (hasTexture[i]) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model->textureID[i]);
            glUniform1i(model->textureUniformID, 0);
            //printf("Set texture\n");
        }
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, model->shapebufferID[i]);
        //printf("Bound the vertex buffer\n");
        glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, model->normalbufferID[i]);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //printf("Starting to draw shape");
        if (hasTexture[i]) {
            //printf("Running texture elements for object %s\n", collisionTag);
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, model->textureCoordsID[i]);
            glVertexAttribPointer(
                                  1,
                                  2,
                                  GL_FLOAT,
                                  GL_FALSE,
                                  0,
                                  (void*)0
                                  );
            glDrawArrays(GL_TRIANGLES, 0, model->pointCount[i] * 3);
            glDisableVertexAttribArray(1);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, model->pointCount[i] * 3);
        }
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(0);
    }
    if (collider != NULL) {
        //infoLock.lock();
        // After drawing the gameobject, draw the collider
        glUseProgram(collider->programID); // grab the programID from the object
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mat4 MVP = vpMatrix * translateMatrix * scaleMatrix * rotateMatrix;
        glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);
        //infoLock.unlock();
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, collider->shapebufferID[0]);
        //printf("Bound the vertex buffer\n");
        glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        glDrawArrays(GL_TRIANGLES, 0, collider->pointCount[0] * 3);
        glDisableVertexAttribArray(0);
    }
    infoLock.unlock();
}
// Returns a pointer to the gameObject's polygon data (contains vertices)
// for the shape.
polygon* GameObject::getModel() {
    return model;
}

void GameObject::deleteTextures() {
    for (int i = 0; model->numberOfObjects; i++) {
        if (hasTexture[i]) {
            glDeleteTextures(1, &textureID);
            hasTexture[i] = false;
        }
    }
}
void GameObject::setVPMatrix(mat4 VPMatrix) {
    vpMatrix = VPMatrix;
}
int GameObject::getCameraID() {
    return currentCamera;
}
vec3 GameObject::getPos(vec3 offset) {
    return pos + offset;
}
vec3 GameObject::getPos() {
    return pos;
}
void GameObject::setLuminance(GLfloat luminanceValue) {
    luminance = luminanceValue;
}
mutex *GameObject::getLock() {
    return &infoLock;
}

/*
 getCollision takes two GameObjects, (GameObject *)object1 and
 (GameObject *)object2 and checks if they are colliding with one another.
 Function will return -1 if object1 or object2 are missing collider objects.
 Otherwise, function will return 0 when no collision is happening and 1
 if the two objects are colliding.
 */
int GameObject::getCollision(GameObject *object1, GameObject *object2) {
    vec3 center = vec3(0.0f, 0.0f, 0.0f);
    // Check for collision objects on object1 and object2
    if (object1 == NULL || object2 == NULL || object1->collider == NULL
        || object2->collider == NULL) {
        return -1;
    }
    // Assume rectangular box collider
    return 0;
}

/* GameCamera objects should be created with a create camera function inside of
 the GameInstance class.
 */

// Camera will follow around a given target.
void GameCamera::configureCamera(cameraInfo camInfo) {
    aspectRatio = camInfo.viewAspectRatio;
    nearClipping = camInfo.viewNearClipping;
    farClipping = camInfo.viewFarClipping;
    offset = camInfo.offset;
    target = camInfo.objTarget;
    cameraAngle = camInfo.viewCameraAngle;
    return;
}
GameObject* GameCamera::getTarget() {
    return target;
}
vec3 GameCamera::getOffset() {
    return offset;
}
mat4 GameCamera::getVPMatrix() {
    return VPmatrix;
}
// Call updateCamera every frame to update VP matrix
void GameCamera::updateCamera() {
    if (target == NULL) {
        cerr << "Error: Unable to update camera! Camera target is NULL!\n";
        return;
    }
    mat4 viewMatrix = lookAt(target->getPos(offset), target->getPos(), vec3(0,1,0));
    mat4 projectionMatrix = perspective(radians(cameraAngle), aspectRatio, nearClipping, farClipping);
    VPmatrix = projectionMatrix * viewMatrix;
}
// If you want to change the offset inside of the game
void GameCamera::setOffset(vec3 newOffset) {
    offset = newOffset;
}
void GameCamera::setTarget(GameObject *targetObject) {
    target = targetObject;
}
