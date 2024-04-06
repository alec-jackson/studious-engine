/**
 * @file OpenGlGfxController.cpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <OpenGlGfxController.hpp>

/**
 * @brief Generates a buffer in the OpenGL context
 * 
 * @param bufferId unsigned int to store new buffer ID created in OpenGL context
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::generateBuffer(GLuint *bufferId) {
    printf("OpenGlGfxController::generateBuffer: bufferId %p\n", bufferId);
    glGenBuffers(1, bufferId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::generateBuffer: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Binds a buffer to the current OpenGL context
 * 
 * @param bufferId ID of buffer to bind (needs to be generated first via generateBuffer)
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::bindBuffer(GLuint bufferId) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::bindBuffer: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Sends data to the currently bound buffer in the OpenGL context. This transfers data from the application
 * side (c++ studious) to the GPU side (OpenGL in this case).
 * 
 * @param size Size of the data array
 * @param data The data array write to the OpenGL buffer
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::sendBufferData(size_t size, void *data) {
    printf("OpenGlGfxController::sendBufferData: size %lu data %p\n", size, data);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::sendBufferData: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Copies texture data to the currently bound texture buffer.
 * 
 * @param width of the texture to send
 * @param height of the texture to send
 * @param format of the texture
 * @param data pixel data to send to the GPU
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::sendTextureData(GLuint width, GLuint height, TexFormat format, void *data) {
    auto texFormat = GL_RGB;
    switch (format) {
        case RGBA:
            texFormat = GL_RGBA;
            break;
        case RGB:
            texFormat = GL_RGB;
            break;
        case BITMAP:
            texFormat = GL_RED;  // Just need a single color
            break;
        default:
            fprintf(stderr, "OpenGlGfxController::sendTextureData: Unknown texture format %d\n", format);
            return GFX_FAILURE(GLuint);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, texFormat, width, height, 0, texFormat, GL_UNSIGNED_BYTE, data);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::sendTextureData: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Generates mipmaps for the currently bound texture
 * 
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::generateMipMap() {
    glGenerateMipmap(GL_TEXTURE_2D);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::generateMipMap: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Generates a new texture and writes the new texture ID to the textureId variable passed in.
 * 
 * @param textureId assigned to newly created texture in OpenGL context.
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::generateTexture(GLuint *textureId) {
    glGenTextures(1, textureId);

    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::generateTexture: Error: %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Cleans up OpenGL artifacts. Currently a WIP.
 * 
 * @return GfxResult<GLint> OK result containing the number of deleted programs.
 */
GfxResult<GLint> OpenGlGfxController::cleanup() {
    auto deletedPrograms = 0;
    for (auto it = programIdList_.begin(); it != programIdList_.end(); ++it) {
        glDeleteProgram(*it);
        deletedPrograms++;
    }
    // glDeleteVertexArrays(1, &vertexArrayId_);
    return GfxResult<GLint>(GfxApiResult::OK, deletedPrograms);
}

/**
 * @brief Gets a programId for a specific index in the programIdList.
 * 
 * @param index in the programId list
 * @return GfxResult<GLuint> OK with returned program ID when successful; FAILURE otherwise.
 */
GfxResult<GLuint> OpenGlGfxController::getProgramId(uint index) {
    // Boundary check index
    if (index > programIdList_.size()) {
        return GfxResult<GLuint>(GfxApiResult::FAILURE, UINT_MAX);
    }
    return GfxResult<GLuint>(GfxApiResult::OK, programIdList_[index]);
}

/**
 * @brief Compiles shaders and adds them to the programId list for this GfxController.
 * 
 * @param vertexShader Path to the vertexShader to compile on the system
 * @param fragmentShader Path to the fragmentShader to compile on the system
 * @return GfxResult<GLuint> OK with the newly created programId; FAILURE otherwise.
 */
GfxResult<GLuint> OpenGlGfxController::loadShaders(string vertexShader, string fragmentShader) {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    int logLength;
    GLint success = GL_FALSE;
    string vertShader, fragShader, tempLine;
    ifstream file;
    file.open(vertexShader);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Error: Cannot open file " << vertexShader << "!\n";
        return GfxResult<GLuint>(GfxApiResult::FAILURE, UINT_MAX);
    }
    while (getline(file, tempLine)) {
        vertShader.append(tempLine + '\n');
    }
    cout << "Now compiling " << vertexShader << "...\n";
    vector<const GLchar *> vertShaderInfo(1);
    vertShaderInfo[0] = reinterpret_cast<const GLchar *>(vertShader.c_str());
    glShaderSource(vertexShaderID, 1, &vertShaderInfo[0], NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength) {
        vector<char> errorLog(logLength + 1);
        glGetShaderInfoLog(vertexShaderID, logLength, NULL, &errorLog[0]);
        cerr << &errorLog[0] << "\n";
    }
    file.close();
    success = GL_FALSE;
    file.open(fragmentShader);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Error: Cannot open file " << fragmentShader << "!\n";
        return GfxResult<GLuint>(GfxApiResult::FAILURE, UINT_MAX);
    }
    while (getline(file, tempLine)) {
        fragShader.append(tempLine + '\n');
    }
    cout << "Now compiling " << fragmentShader << "...\n";
    vector<const GLchar *>fragShaderInfo(1);
    fragShaderInfo[0] = reinterpret_cast<const GLchar *>(fragShader.c_str());
    glShaderSource(fragmentShaderID, 1, &fragShaderInfo[0], NULL);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength) {
        vector<char> errorLog(logLength + 1);
        glGetShaderInfoLog(vertexShaderID, logLength, NULL, &errorLog[0]);
        cerr << &errorLog[0] << "\n";
    }
    file.close();
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderID);
    glAttachShader(programId, fragmentShaderID);
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength) {
        vector<char> errorLog(logLength + 1);
        glGetProgramInfoLog(programId, logLength, NULL, &errorLog[0]);
        cerr << &errorLog[0] << "\n";
    }
    glDetachShader(programId, vertexShaderID);
    glDetachShader(programId, fragmentShaderID);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    // Add programId to programIdList
    programIdList_.push_back(programId);

    return GfxResult<GLuint>(GfxApiResult::OK, programId);
}

/**
 * @brief Gets the location of a variable in a shader
 * 
 * @param programId program to search for variable inside of.
 * @param variableName name of variable to find in shader.
 * @return GfxResult<GLint> OK with variableId; FAILURE otherwise.
 */
GfxResult<GLint> OpenGlGfxController::getShaderVariable(GLuint programId, const char *variableName) {
    auto varId = glGetUniformLocation(programId, variableName);
    auto result = GfxResult<GLint>(GfxApiResult::FAILURE, varId);
    if (varId == -1) result = GfxResult<GLint>(GfxApiResult::OK, varId);
    return result;
}

/**
 * @brief Has a list of OpenGL specific commands that update once per frame. This is not part of the GfxController
 * interface.
 * 
 */
void OpenGlGfxController::updateOpenGl() {
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief Runs through any updates.
 * 
 */
void OpenGlGfxController::update() {
    updateOpenGl();
}

/**
 * @brief Initialize the OpenGL context.
 * 
 * @return GfxResult<GLint> OK if successful; FAILURE otherwise
 */
GfxResult<GLint> OpenGlGfxController::init() {
    cout << "OpenGlGfxController::init" << endl;
    if (glewInit() != GLEW_OK) {
        cerr << "Error: Failed to initialize GLEW!\n";
        return GFX_FAILURE(GLint);
    }
    // Set pixel storage alignment mode for font loading
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return GFX_OK(GLint);
}

/**
 * @brief Sets the current program (shader) in the OpenGL context.
 * 
 * @param programId created by OpenGL for a set of shaders (program).
 * @return GfxResult<GLuint> OK if successful, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::setProgram(GLuint programId) {
    glUseProgram(programId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::setProgram: Error: %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Sends a float to a corresponding variable inside of a program (shader)
 * 
 * @param variableId of the variable inside of the program to send data to
 * @param data to write to variable
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::sendFloat(GLuint variableId, GLfloat data) {
    glUniform1f(variableId, data);
    return GFX_OK(GLuint);
}

/**
 * @brief Sends a 3D vector of floats to a shader variable
 * 
 * @param variableId variableId to write data to
 * @param count Number of float vectors to send
 * @param data Raw data to send to GPU
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) {
    glUniform3fv(variableId, count, data);
    return GFX_OK(GLuint);
}

/**
 * @brief Sets the rendering mode for triangles.
 * @see RenderMode enum for rendering options.
 * 
 * @param mode method for rendering triangles.
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::polygonRenderMode(RenderMode mode) {
    auto result = GFX_OK(GLuint);
    switch (mode) {
        case RenderMode::POINT:
            glPolygonMode(GL_FRONT, GL_POINT);
            break;
        case RenderMode::LINE:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case RenderMode::FILL:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        default:
            fprintf(stderr, "OpenGlGfxController::polygonRenderMode: Unsupported render mode %d", mode);
            result = GFX_FAILURE(GLuint);
            break;
    }
    return result;
}

/**
 * @brief Sends a 4x4 matrix of floats to the GPU memory for a variable in the current program.
 * 
 * @param variableId Variable to write data to
 * @param count Number of matrices to write - generally just one
 * @param data Raw data to send over to program variable.
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::sendFloatMatrix(GLuint variableId, GLsizei count, GLfloat *data) {
    glUniformMatrix4fv(variableId, count, GL_FALSE, data);
    return GFX_OK(GLuint);
}

/**
 * @brief Sends an integer to the GPU memory for a variable in the current program.
 * 
 * @param variableId Variable identifier in the current program
 * @param data Raw data to copy to GPU
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::sendInteger(GLuint variableId, GLint data) {
    glUniform1i(variableId, data);
    return GFX_OK(GLuint);
}

/**
 * @brief Binds a texture to the current OpenGL context.
 * 
 * @param textureId ID of texture to bind.
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlGfxController::bindTexture(GLuint textureId) {
    // Use texture unit zero - nothing fancy
    glActiveTexture(GL_TEXTURE0);
    // Binds the specific textureId to a GL_TEXTURE_2D - might only need to do once?
    glBindTexture(GL_TEXTURE_2D, textureId);
    return GFX_OK(GLuint);
}

/**
 * @brief Binds a VAO object in the current OpenGL context
 * @note The name of this method will most likely change other GFX backends are added.
 * 
 * @param vao VAO ID to bind
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::bindVao(GLuint vao) {
    glBindVertexArray(vao);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        /// @todo When a logger is added, add OpenGL error log debugging
        fprintf(stderr, "OpenGlGfxController::bindVao: Error: %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Enables/Disables an OpenGL capability in the current context.
 * 
 *        Usage: setFeature(GL_CULL_FACE, true)
 * 
 * will enable the GL_CULL_FACE capability.
 * 
 * @param capabilityId ID of the capability to toggle on/off.
 * @param enabled Whether or not to enable/disable the capability
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::setCapability(GfxCapability capability, bool enabled) {
    auto capabilityId = 0;
    switch (capability) {
        case CULL_FACE:
            capabilityId = GL_CULL_FACE;
            break;
        default:
            printf("OpenGlGfxController::setCapability: Unknown capability %d\n",
                static_cast<int>(capability));
            return GFX_FAILURE(GLuint);
    }
    enabled ? glEnable(capabilityId) : glDisable(capabilityId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::setCapability: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Initializes a VAO object
 * 
 * @param vao to initialize
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::initVao(GLuint *vao) {
    glGenVertexArrays(1, vao);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::initVao: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Deletes textures from the OpenGL context.
 * 
 * @param tId texture ID to delete in the OpenGL context.
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::deleteTextures(GLuint *tId) {
    glDeleteTextures(1, tId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::deleteTextures: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Updates a VBO buffer with a set of vertices
 * 
 * @param vertices Vector of vertex data to copy to VBO object
 * @param vbo identifier for the buffer to write data into
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::updateBufferData(const vector<GLfloat> &vertices, GLuint vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertices.size(), &vertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::updateBufferData: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Sets a texture parameter for the texture currently bound in the OpenGL context
 * 
 * @param param parameter to update @see TexParam in GfxController
 * @param val value to set for the given parameter
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::setTexParam(TexParam param, TexVal val) {
    // Convert Studious GFX enums to OpenGL enums
    auto glParam = 0;
    auto glVal = 0;
    switch (param) {
        case WRAP_MODE_S:
            glParam = GL_TEXTURE_WRAP_S;
            break;
        case WRAP_MODE_T:
            glParam = GL_TEXTURE_WRAP_T;
            break;
        case MINIFICATION_FILTER:
            glParam = GL_TEXTURE_MIN_FILTER;
            break;
        case MAGNIFICATION_FILTER:
            glParam = GL_TEXTURE_MAG_FILTER;
            break;
        case MIPMAP_LEVEL:
            glParam = GL_TEXTURE_MAX_LEVEL;
            break;
        default:
            printf("OpenGlGfxController::setTexParam: Unknown parameter option for OpenGL: %d\n",
                static_cast<int>(param));
            return GFX_FAILURE(GLuint);
    }
    switch (val.type()) {
        case CLAMP_TO_EDGE:
            glVal = GL_CLAMP_TO_EDGE;
            break;
        case GFX_LINEAR:
            glVal = GL_LINEAR;
            break;
        case NEAREST_MIPMAP:
            glVal = GL_NEAREST_MIPMAP_LINEAR;
            break;
        case NEAREST_NEIGHBOR:
            glVal = GL_NEAREST;
            break;
        case CUSTOM:
            glVal = val.data();
            break;
        default:
            printf("OpenGlGfxController::setTexParam: Unknown parameter option for OpenGL: %d\n",
                static_cast<int>(param));
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, glParam, glVal);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::setTexParam: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Enables a vertex attribute array and configures the vertex attribute pointer
 * 
 * @param layout The layout set in the OpenGL shader
 * @param size The per-object size. A 3D vertex would have a size of 3, because each point is made of 3 floats
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::enableVertexAttArray(GLuint layout, size_t size) {
    glEnableVertexAttribArray(layout);
    glVertexAttribPointer(
        layout,                     // layout in shader
        size,                       // size
        GL_FLOAT,                   // type
        GL_FALSE,                   // normalized?
        size * sizeof(GLfloat),     // stride
        0);                         // array buffer offset
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::enableVertexAttArray: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Disables a vertex attribute array in the OpenGL context.
 * 
 * @param layout The layout of the vertex attribute array to disable in the current program.
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::disableVertexAttArray(GLuint layout) {
    glDisableVertexAttribArray(layout);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::disableVertexAttArray: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Draws triangles using currently bound textures/buffers.
 * 
 * @param size The number of vertices to render
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlGfxController::drawTriangles(GLuint size) {
    glDrawArrays(GL_TRIANGLES, 0, size);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::drawTriangles: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Clears buffers in the OpenGL context. Common uses are COLOR buffers (framebuffer) or
 * DEPTH buffers.
 * 
 * @param clearMode what should be cleared @see GfxClearMode
 */
void OpenGlGfxController::clear(GfxClearMode clearMode) {
    auto clearVal = 0;
    switch (clearMode) {
        case COLOR:
            clearVal = GL_COLOR_BUFFER_BIT;
            break;
        case DEPTH:
            clearVal = GL_DEPTH_BUFFER_BIT;
            break;
        default:
            printf("OpenGlGfxController::clear: Unknown clearMode %d\n", clearMode);
            return;
    }
    glClear(clearVal);
}

