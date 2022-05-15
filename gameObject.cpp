#include "gameObject.hpp"

/*
 (void) configureGameObject takes some (gameObjectInfo) objectInfo as an
 argument and configures the current GameObject instance using that information.
 Before a GameObject can be rendered in the scene, it MUST first be configured
 using this method. See documentation in gameObject.hpp for details on the
 gameObjectInfo struct and its members.

 (void) configureGameObject does not return any values.
*/
void GameObject::configureGameObject(gameObjectInfo objectInfo) {
    programID = objectInfo.characterModel->programID;
    collider.collider = NULL; // Default to not having a collider
    luminance = 1.0f;
    rollOff = 0.9f; // Rolloff describes the intensity of the light dropoff
    directionalLight = vec3(0,0,0);
    orthographic = false;
    model = objectInfo.characterModel;
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
    scale = objectInfo.scale;
    rot = objectInfo.rot;
    pos = objectInfo.pos;
    scaleMatrix = glm::scale(vec3(scale, scale, scale));
    translateMatrix = glm::translate(mat4(1.0f), objectInfo.pos);
    rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(rot[0]),
            vec3(1,0,0))  *glm::rotate(mat4(1.0f), glm::radians(rot[1]),
            vec3(0,1,0))  *glm::rotate(mat4(1.0f), glm::radians(rot[2]),
            vec3(0,0,1));
    collider.collisionTag = objectInfo.collisionTagName;
    // Grab IDs for shared variables between app and program (shader)
    rotateID = glGetUniformLocation(programID, "rotate");
    scaleID = glGetUniformLocation(programID, "scale");
    translateID = glGetUniformLocation(programID, "move");
    vpID = glGetUniformLocation(programID, "VP");
    hasTextureID = glGetUniformLocation(programID, "hasTexture");
    directionalLightID = glGetUniformLocation(programID, "directionalLight");
    luminanceID = glGetUniformLocation(programID, "luminance");
    rollOffID = glGetUniformLocation(programID, "rollOff");
    if (collider.collider != NULL) {
        MVPID = glGetUniformLocation(collider.collider->programID, "MVP");
    }
    vpMatrix = mat4(1.0f); // Default VP matrix to identity matrix
    configured = true; // Set configured flag to true for rendering
}

/*
 (string) getColliderTag returns the collisionTag string from the current
 GameObject instance.
*/
string GameObject::getColliderTag(void) {
    lockObject();
    if (collider.collisionTag.empty()) {
        cerr << "GameObject was not assigned a collider tag!\n";
    }
    string curTag = collider.collisionTag;
    unlockObject();
    return curTag;
}

/*
 (colliderInfo) getCollider returns the (colliderInfo) collider associated with
 the current GameObject.
*/
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

/*
 (void) setRotation takes a (vec3) rotation describing the desired XYZ rotation
 of the GameObject and sets the GameObject's (vec3) rot instance variable to the
 given argument. When successful, the GameObject should be rotated on the next
 draw.

 (void) setRotation does not return any values.
*/
void GameObject::setRotation(vec3 rotation) {
    rot = rotation;
}

/*
 (void) setOrtho takes a true/false (bool) ortho value and sets the GameObject's
 orthographic value to that boolean.

 (void) setOrtho does not return any values.
*/
void GameObject::setOrtho(bool ortho) {
    orthographic = ortho;
}

/*
 (void) setPos takes a (vec3) position argument and sets the GameObject's
 (vec3) pos instance variable equal to the given position. When successful, the
 GameObject should be rotated on the next draw.

 (void) setPos does not return any values.
*/
void GameObject::setPos(vec3 position) {
    pos = position;
}

/*
 (void) setScale takes a (GLfloat) uniformScale as an argument and sets the
 (GLfloat) scale variable inside of the current GameObject instance equal to the
 given uniformScale value. When successful, the uniform scale should be applied
 to the GameObject on the new draw.

 (void) setScale does not return any values.
*/
void GameObject::setScale(GLfloat uniformScale) {
    scale = uniformScale;
}

/*
 (GLfloat) getScale returns the (GLfloat) scale value from the current
 GameObject.
*/
GLfloat GameObject::getScale() {
    return scale;
}

/*
 (void) setDirectionalLight takes a (vec3) newLight that describes the direction
 that the light is coming from and sets that to the GameObject's
 (vec3) directionalLight value.

 (void) setDirectionalLight does not return any values.
*/
void GameObject::setDirectionalLight(vec3 newLight) {
    directionalLight = newLight;
}

/*
 (void) setProgramID takes a (GLuint) shaderID and sets the GameObject's
 programID instance variable to shaderID. This function should NOT be used when
 a GameObject is using a model loaded from modelImport functions, because those
 already contain programID's tied to their polygon objects. This function
 should mainly be used with sprites or 2D elements like text.

 (void) setProgramID does not return any values.
*/
void GameObject::setProgramID(GLuint shaderID) {
    programID = shaderID;
}

/*
 (void) drawShape draws the current GameObject inside of the scene. This is the
 method that interacts with OpenGL the most (just to let you know). Method
 walks through all of the objects inside of the current model and renders them
 one by one in the for loop below.

 (void) drawShape does not return any values.
*/
void GameObject::drawShape() {
    // Draw each shape individually
    if (!configured) {
        cerr << "GameObject with tag " << collider.collisionTag
            << " has not been configured yet!\n";
        return;
    }
    for (int i = 0; i < model->numberOfObjects; i++) {
        glUseProgram(programID);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // Update our model transformation matrices
        translateMatrix = glm::translate(mat4(1.0f), pos);
        rotateMatrix = glm::rotate(mat4(1.0f), glm::radians(rot[0]),
                vec3(1,0,0))  *glm::rotate(mat4(1.0f), glm::radians(rot[1]),
                vec3(0,1,0))  *glm::rotate(mat4(1.0f), glm::radians(rot[2]),
                vec3(0,0,1));
        scaleMatrix = glm::scale(vec3(scale, scale, scale));
        // Send our shared variables over to our program (shader)
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
        }
        // Actually start drawing polygons :)
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, model->shapebufferID[i]);
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
        if (hasTexture[i]) {
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
    if (collider.collider != NULL) {
        // After drawing the gameobject, draw the collider
        glUseProgram(collider.collider->programID); // grab the programID from the object
        glDisable(GL_CULL_FACE); // Just do it
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mat4 MVP = vpMatrix * translateMatrix * scaleMatrix * rotateMatrix;
        glUniformMatrix4fv(MVPID, 1, GL_FALSE, &MVP[0][0]);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, collider.collider->shapebufferID[0]);
        glVertexAttribPointer(
                              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                              3,                  // size
                              GL_FLOAT,           // type
                              GL_FALSE,           // normalized?
                              0,                  // stride
                              (void*)0            // array buffer offset
                              );
        glDrawArrays(GL_TRIANGLES, 0, collider.collider->pointCount[0] * 3);
        glDisableVertexAttribArray(0);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

/*
 (polygon *) getModel returns the model associated with the current GameObject
 instance.
*/
polygon* GameObject::getModel() {
    return model;
}

/*
 (void) deleteTextures loops through the GameObject's models and deletes
 the textures bound in OpenGL to those models if present.

 (void) deleteTextures does not return any values.
*/
void GameObject::deleteTextures() {
    for (int i = 0; model->numberOfObjects; i++) {
        if (hasTexture[i]) {
            glDeleteTextures(1, &textureID);
            hasTexture[i] = false;
        }
    }
}

/*
 (void) setVPMatrix takes a (mat4) VPMatrix and updates the current GameObject's
 (mat4) vpMatrix instance variable to the argument. The VP matrix is used for
 creating the MVP matrix which contains all of the linear transformations we
 need to get our image looking correct. The VP matrix should be generated by the
 GameCamera in the scene.

 (void) setVPMatrix does not return any values.
*/
void GameObject::setVPMatrix(mat4 VPMatrix) {
    vpMatrix = VPMatrix;
}

/*
 (int) getCameraID returns the ID of the current GameCamera that this current
 GameObject instance is using.
*/
int GameObject::getCameraID() {
    return currentCamera;
}

/*
 (vec3) getPos takes a (vec3) offset and returns (vec3) pos + offset, which is
 a 3D vector of the position plus the offset.
*/
vec3 GameObject::getPos(vec3 offset) {
    return pos + offset;
}

/*
 (vec3) getPos returns the (vec3) pos position of the current GameObject.
*/
vec3 GameObject::getPos() {
    return pos;
}

/*
 (GLuint) getProgramID returns the current programID value associated with the
 current GameObject.
*/
GLuint GameObject::getProgramID() {
    return programID;
}

/* [DEPRECATED]
 (void) setLuminance uses the given (GLfloat) luminanceValue to set the
 GameObject's (GLfloat) luminance value, which controls the intensity of the
 light on the object.

 (void) setLuminance does not return any values.
*/
void GameObject::setLuminance(GLfloat luminanceValue) {
    luminance = luminanceValue;
}

/*
 (void) setCollider sets the collider tag for the current GameObject.

 (void) setCollider does not return any values.
*/
void GameObject::setCollider(string coll) {
    collider.collisionTag = coll;
}

/*
 (mat4) getVPMatrix returns the current VP matrix for the GameObject.
*/
mat4 GameObject::getVPMatrix() {
    return vpMatrix;
}

/*
 (bool) isOrtho returns the value of the orthographic boolean in the GameObject
 class. This function determines whether the object will be rendered
 orthographically (should be rendered this way for UI text).
*/
bool GameObject::isOrtho() {
    return orthographic;
}

/* DO NOT USE - outdated locking mechanism. Use sceneLock instead
 (int) lockObject uses the GameObject's internal infoLock mutex to obtain the
 lock for the current GameObject.

 (int) lockObject returns 0 on success.
*/
int GameObject::lockObject() {
    infoLock.lock();
    return 0;
}

/* DO NOT USE - outdated locking mechanism. Use sceneLock instead
 (int) unlockObject uses the GameObject's internal infoLock mutex to obtain the
 lock for the current GameObject.

 (int) unlockObject returns 0 on success.
*/
int GameObject::unlockObject() {
    infoLock.unlock();
    return 0;
}

/*
 (bool) min_func takes some (float) a and (float) b values and returns the
 boolean (a < b), so true if a is less than b, otherwise false.
*/
bool min_func(float a, float b) {
    return a < b;
}

/*
 (bool) max_func takes some (float) a and (float) b values and returns the
 boolean (a > b), so true if a is greater than b, otherwise false.
*/
bool max_func(float a, float b) {
    return a > b;
}

/*
 (GLfloat) getVert takes some (vector<GLfloat>) vertices, takes an (int) axis
 defining the axis we're looking at specifically (X, Y, Z -> 0, 1, 2
 respectively). The test for the returned vertex is defined in the passed in
 (bool (*test)(float a, float b)) function. An example parameter function can be
 seen in the built in max_func and min_func functions.

 (GLfloat) returns the desired vertex point on success. On error, 0.0f is
 returned and an error message is printed to stderr.
*/
GLfloat GameObject::getVert(vector<GLfloat> vertices, int axis,
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

/*
 (int) createCollider takes an (int) shaderID and creates a box collider that
 tightly fits around the current GameObject. This process should be fully
 automatic, so just call this method on any object that you want to create a
 collider for and it should work just fine. The cube collider that is created
 uses the minimum and maxmimum XYZ coordinates of the GameObject and creates a
 box that fits those bounds.

 (int) createCollider returns 0 upon success. On error, -1 is returned and an
 error message is printed to stderr.
*/
int GameObject::createCollider(int shaderID) {
    cout << "Building collider for " << collider.collisionTag << endl;
    GLfloat min[3] = {999, 999, 999}, tempMin[3] = {999, 999, 999};
    GLfloat max[3] = {-999, -999, -999}, tempMax[3] = {-999, -999, -999};
    if (model == NULL) {
        cerr <<
            "Error: Cannot create collider because GameObject model is missing"
            << endl;
        return -1;
    }
    vector<vector<GLfloat>>::iterator it;
    // Go through objects and get absolute min/max points
    for (it = model->vertices.begin(); it != model->vertices.end(); ++it) {
        for (int i = 0; i < 3; i++) {
            tempMin[i] = getVert((*it), i, min_func);
            tempMax[i] = getVert((*it), i, max_func);
            if (tempMin[i] < min[i]) {
                min[i] = tempMin[i];
            }
            if (tempMax[i] > max[i]) {
                max[i] = tempMax[i];
            }
        }
    }
    // Create new polygon object for collider
    collider.collider = new polygon();
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
    collider.collider->vertices.push_back(colliderVertices);
    collider.collider->textureID.push_back(UINT_MAX);
    collider.collider->textureCoordsID.push_back(UINT_MAX);
    collider.collider->shapebufferID.push_back(0);
    collider.collider->pointCount.push_back(108);
    collider.collider->programID = shaderID;
    glGenBuffers(1, &(collider.collider->shapebufferID[0]));
    glBindBuffer(GL_ARRAY_BUFFER, collider.collider->shapebufferID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 108,
        &(collider.collider->vertices[0][0]), GL_STATIC_DRAW);
    // Set the correct center points
    for (int i = 0; i < 3; i++) {
        collider.center[i] = max[i] - ((abs(max[i] - min[i])) / 2);
    }
    collider.center[3] = 1; // SET W!!!
    collider.originalCenter = collider.center;
    // Update the offset for the collider to be distance between center and edge
    for (int i = 0; i < 3; i++) {
        collider.minPoints[i] = min[i];
    }
    collider.minPoints[3] = 1; // SET W!!!
    return 0;
}

/*
 (int) initializeText takes an (textObjectInfo) info argument and creates a new
 GameObjectText object in the current GameInstance. The textObjectInfo struct
 contains information like the message to be rendered to the screen, the shader
 ID to use for rendering, and the font to use for the text.

 (int) initializeText returns 0 on success, otherwise -1 is returned and an
 error is printed to stderr.
*/
int GameObjectText::initializeText(textObjectInfo info) {
    mat4 projection = ortho(0.0f, static_cast<float>(1280), 0.0f, static_cast<float>(720));
    setProgramID(info.programID);
    cout << "Initializing text with message " << info.message << endl;
    setCollider("Text");
    message = info.message;
    setPos(vec3(300.0f, 300.0f, 0.0f));
    setScale(1.0f);
    glUseProgram(getProgramID()); // Load text shader
    glUniformMatrix4fv(glGetUniformLocation(getProgramID(),
        "projection"), 1, GL_FALSE, &projection[0][0]);
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return -1;
    }
    FT_Face face;
    if (FT_New_Face(ft, "misc/fonts/AovelSans.ttf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return -1;
    }
    else {
        FT_Set_Pixel_Sizes(face, 0, 48);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for (unsigned char c = 0; c < 128; c++)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                cout << "ERROR::FREETYTPE: Failed to load Glyph\n";
                continue;
            }
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                         GL_TEXTURE_2D,
                         0,
                         GL_RED,
                         face->glyph->bitmap.width,
                         face->glyph->bitmap.rows,
                         0,
                         GL_RED,
                         GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer
                         );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    setOrtho(true);
    return 0;
}

/*
 (void) drawText draws the current message for the GameObjectText onto the
 screen. Current GameObjectText is locked when rendering the text, and unlocked
 when finished.

 (void) drawText does not return any values.
*/
void GameObjectText::drawText() {
    lockObject();
    glClear(GL_DEPTH_BUFFER_BIT);
    vec3 color = vec3(1.0f);
    float scale = getScale();
    int x = getPos().x, y = getPos().y;
    glUseProgram(getProgramID());
    glUniform3f(glGetUniformLocation(getProgramID(), "textColor"),
        color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    std::string::const_iterator c;
    for (c = message.begin(); c != message.end(); c++)
    {
        Character ch = characters[*c];
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    unlockObject();
}

/*
 (string) getMessage returns the (string) message being rendered to the
 current game scene.
*/
string GameObjectText::getMessage() {
    return message;
}

/*
 (void) setMessage takes a (string) phrase and sets the GameObjectText's
 current (string) message variable to the given phrase.

 (void) setMessage does not return any values.
*/
void GameObjectText::setMessage(string phrase) {
    lockObject();
    message = phrase;
    unlockObject();
}

/*
 (void) configureCamera takes some (cameraInfo) camInfo and constructs a camera
 based on the values of the members in the camInfo struct. See the documentation
 for details on using the camInfo struct inside of gameObject.hpp. When creating
 a new GameCamera in a scene, it must first be configured before it can be used.

 (void) configureCamera does not return any values.
*/
void GameCamera::configureCamera(cameraInfo camInfo) {
    aspectRatio = camInfo.viewAspectRatio;
    nearClipping = camInfo.viewNearClipping;
    farClipping = camInfo.viewFarClipping;
    offset = camInfo.offset;
    target = camInfo.objTarget;
    cameraAngle = camInfo.viewCameraAngle;
}

/*
 (GameObject *) getTarget returns a pointer to the GameObject that the current
 GameCamera is targetting.
*/
GameObject* GameCamera::getTarget() {
    return target;
}

/*
 (vec3) getOffset returns the current (vec3) offset of the GameCamera relative
 to the target GameObject. A critical section was added when obtaining the
 offset due to the high possibility of a change occuring when reading.
*/
vec3 GameCamera::getOffset() {
    lockObject();
    vec3 ofs = offset;
    unlockObject();
    return ofs;
}

/*
 (mat4) getVPMatrix returns the current VPmatrix contained within the
 GameCamera. A critical section was added when obtaining the VPmatrix due to the
 high possibility of a change occuring when reading.
*/
mat4 GameCamera::getVPMatrix() {
    lockObject();
    mat4 VPmat = VPmatrix;
    unlockObject();
    return VPmat;
}

/*
 (void) updateCamera updates the (mat4) VPmatrix inside of the GameCamera
 instance. updateCamera should be called whenever a change has been made to the
 camera. Changes made to the VPmatrix are protected by the camera's
 inherited (mutex) infoLock.

 (void) updateCamera does not return any values.
*/
void GameCamera::updateCamera() {
    if (target == NULL) {
        cerr << "Error: Unable to update camera! Camera target is NULL!\n";
        return;
    }
    // Create critical section here to prevent race conditions
    lockObject();
    mat4 viewMatrix = lookAt(target->getPos(offset), target->getPos(),
        vec3(0,1,0));
    mat4 projectionMatrix = perspective(radians(cameraAngle), aspectRatio,
        nearClipping, farClipping);
    VPmatrix = projectionMatrix * viewMatrix;
    unlockObject();
}

/*
 (void) setOffset takes a (vec3) newOffset as an argument and sets the
 GameCamera's (vec3) offset to the newOffset.

 (void) setOffset does not return any values.
*/
void GameCamera::setOffset(vec3 newOffset) {
    offset = newOffset;
}

/*
 (void) setTarget takes a (GameObject *) targetObject as an argument and sets
 the current GameCamera's object to the new targetObject.

 (void) setTarget does not return any values.
*/
void GameCamera::setTarget(GameObject *targetObject) {
    if (targetObject == NULL) cout << "NULL detected!\n";
    target = targetObject;
}

/*
 (void) setAspectRatio takes a (GLfloat) ratio from the user and sets the
 GameCamera's (GLfloat) aspectRatio value to ratio. The value given to
 setAspectRatio should be equal to window width / window height. The
 aspectRatio value should be expected to change whenever the window size
 changes.

 (void) setAspectRatio does not return any values.
*/
void GameCamera::setAspectRatio(GLfloat ratio) {
    aspectRatio = ratio;
}
