/**
 * @file OpenGlEsGfxController.cpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <OpenGlEsGfxController.hpp>

/**
 * @brief Generates a buffer in the OpenGL context
 * 
 * @param bufferId unsigned int to store new buffer ID created in OpenGL context
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlEsGfxController::generateBuffer(GLuint *bufferId) {
#ifdef VERBOSE_LOGS
    printf("OpenGlEsGfxController::generateBuffer: bufferId %p\n", bufferId);
#endif
    glGenBuffers(1, bufferId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::generateBuffer: Error %d\n", error);
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
GfxResult<GLuint> OpenGlEsGfxController::bindBuffer(GLuint bufferId) {
    auto exists = vaoBindData_.find(activeVao_) != vaoBindData_.end();
    if (activeVao_ && exists) {
        // Create a new entry in the vaoBindData_ buffer map
        vaoBindData_[activeVao_][bufferId] = GfxVaoData();
    } else {
        fprintf(stderr, "OpenGlEsGfxController::bindBuffer: No active VAO %d\n", activeVao_);
    }
    activeVbo_ = bufferId;
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
GfxResult<GLuint> OpenGlEsGfxController::sendBufferData(size_t size, void *data) {
#ifdef VERBOSE_LOGS
    printf("OpenGlEsGfxController::sendBufferData: size %lu data %p\n", size, data);
#endif
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::sendBufferData: Error %d\n", error);
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
GfxResult<GLuint> OpenGlEsGfxController::sendTextureData(GLuint width, GLuint height, TexFormat format, void *data) {
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
            fprintf(stderr, "OpenGlEsGfxController::sendTextureData: Unknown texture format %d\n", format);
            return GFX_FAILURE(GLuint);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, texFormat, width, height, 0, texFormat, GL_UNSIGNED_BYTE, data);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::sendTextureData: Error %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Generates mipmaps for the currently bound texture
 * 
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlEsGfxController::generateMipMap() {
#ifdef VERBOSE_LOGS
    printf("OpenGlEsGfxController::generateMipMap: MipMaps unsupported in OpenGL ES 2.0!\n");
#endif
    return GFX_OK(GLuint);
}

/**
 * @brief Generates a new texture and writes the new texture ID to the textureId variable passed in.
 * 
 * @param textureId assigned to newly created texture in OpenGL context.
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlEsGfxController::generateTexture(GLuint *textureId) {
    glGenTextures(1, textureId);

    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::generateTexture: Error: %d\n", error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Cleans up OpenGL artifacts. Currently a WIP.
 * 
 * @return GfxResult<GLint> OK result containing the number of deleted programs.
 */
GfxResult<GLint> OpenGlEsGfxController::cleanup() {
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
GfxResult<GLuint> OpenGlEsGfxController::getProgramId(uint index) {
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
GfxResult<GLuint> OpenGlEsGfxController::loadShaders(string vertexShader, string fragmentShader) {
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
        cerr << vertexShader << ": " << &errorLog[0] << "\n";
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
        cerr << fragmentShader << ": " << &errorLog[0] << "\n";
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

    printf("OpenGlEsGfxController::loadShaders: Created programId %d\n", programId);

    return GfxResult<GLuint>(GfxApiResult::OK, programId);
}

/**
 * @brief Gets the location of a variable in a shader
 * 
 * @param programId program to search for variable inside of.
 * @param variableName name of variable to find in shader.
 * @return GfxResult<GLint> OK with variableId; FAILURE otherwise.
 */
GfxResult<GLint> OpenGlEsGfxController::getShaderVariable(GLuint programId, const char *variableName) {
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
void OpenGlEsGfxController::updateOpenGl() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.2f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::updateOpenGl: Error: %d\n", error);
        return;
    }
}

/**
 * @brief Runs through any updates.
 * 
 */
void OpenGlEsGfxController::update() {
    updateOpenGl();
}

/**
 * @brief Initialize the OpenGL context.
 * 
 * @return GfxResult<GLint> OK if successful; FAILURE otherwise
 */
GfxResult<GLint> OpenGlEsGfxController::init() {
    cout << "OpenGlEsGfxController::init" << endl;
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
GfxResult<GLuint> OpenGlEsGfxController::setProgram(GLuint programId) {
    glUseProgram(programId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::setProgram: programId %d Error: %d\n", programId, error);
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
GfxResult<GLuint> OpenGlEsGfxController::sendFloat(GLuint variableId, GLfloat data) {
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
GfxResult<GLuint> OpenGlEsGfxController::sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) {
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
GfxResult<GLuint> OpenGlEsGfxController::polygonRenderMode(RenderMode mode) {
#ifdef VERBOSE_LOGS
    printf("OpenGlEsGfxController::polygonRenderMode: No special render modes on OpenGL ES 2.0\n");
#endif
    return GFX_OK(GLuint);
}

/**
 * @brief Sends a 4x4 matrix of floats to the GPU memory for a variable in the current program.
 * 
 * @param variableId Variable to write data to
 * @param count Number of matrices to write - generally just one
 * @param data Raw data to send over to program variable.
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlEsGfxController::sendFloatMatrix(GLuint variableId, GLsizei count, GLfloat *data) {
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
GfxResult<GLuint> OpenGlEsGfxController::sendInteger(GLuint variableId, GLint data) {
    glUniform1i(variableId, data);
    return GFX_OK(GLuint);
}

/**
 * @brief Binds a texture to the current OpenGL context.
 * 
 * @param textureId ID of texture to bind.
 * @return GfxResult<GLuint> OK if successful; FAILURE otherwise
 */
GfxResult<GLuint> OpenGlEsGfxController::bindTexture(GLuint textureId) {
    // Use texture unit zero - nothing fancy
    glActiveTexture(GL_TEXTURE0);
    // Binds the specific textureId to a GL_TEXTURE_2D - might only need to do once?
    glBindTexture(GL_TEXTURE_2D, textureId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        /// @todo When a logger is added, add OpenGL error log debugging
        fprintf(stderr, "OpenGlEsGfxController::bindTexture: textureId %u, Error: %d\n", textureId, error);
        return GFX_FAILURE(GLuint);
    }
    return GFX_OK(GLuint);
}

/**
 * @brief Binds a VAO object in the current OpenGL context
 * @note The name of this method will most likely change other GFX backends are added.
 * 
 * @param vao VAO ID to bind
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlEsGfxController::bindVao(GLuint vao) {
    if (vao == UINT_MAX) {
        fprintf(stderr, "OpenGlEsGfxController::bindVao: Bad vao handler\n");
        return GFX_FAILURE(GLuint);
    }
    if (vao != 0) {
        // Check if the VAO bind data exists in the vaoBindData_ map
        auto exists = vaoBindData_.find(vao) != vaoBindData_.end();
        if (exists) {
            // Bind each VBO and enable attribute array pointer
            for (const auto &[vbo, bindData] : vaoBindData_[vao]) {
                // Bind using the VAO data
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glVertexAttribPointer(
                    bindData.layout,            // layout in shader
                    bindData.size,              // size
                    GL_FLOAT,                   // type
                    GL_FALSE,                   // normalized?
                    bindData.size * sizeof(GLfloat),     // stride
                    0);                         // array buffer offset
                glEnableVertexAttribArray(bindData.layout);
            }
        } else {
            printf("OpenGlEsGfxController::bindVao: VAO uninitialized\n");
        }
    } else {
        // If we're clearing the currently bound vao...\
	printf("OpenGlEsGfxController::bindVao: Clearing previous VAO\n");
	for (const auto &[vbo, bindData] : vaoBindData_[vao]) {
            disableVertexAttArray(bindData.layout);
	}
        //disableVertexAttArray(vaoBindData_[vao].layout);
    }
    activeVao_ = vao;
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
GfxResult<GLuint> OpenGlEsGfxController::setCapability(GfxCapability capability, bool enabled) {
    auto capabilityId = 0;
    switch (capability) {
        case CULL_FACE:
            capabilityId = GL_CULL_FACE;
            break;
        default:
            printf("OpenGlEsGfxController::setCapability: Unknown capability %d\n",
                static_cast<int>(capability));
            return GFX_FAILURE(GLuint);
    }
    enabled ? glEnable(capabilityId) : glDisable(capabilityId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::setCapability: Error %d\n", error);
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
GfxResult<GLuint> OpenGlEsGfxController::initVao(GLuint *vao) {
    static int vaoId;
    // Ensure that vaoId does NOT start at zero - increment the first time around
    vaoId = !vaoId ? vaoId++ : vaoId;
    *vao = vaoId++;
    // Create a new entry in the vaoBindData_ map
    vaoBindData_[*vao];// = vector<GfxVaoData>();
#ifdef VERBOSE_LOGS
    printf("OpenGlEsGfxController::initVao: Created vao %d\n", *vao);
#endif
    return GFX_OK(GLuint);
}

/**
 * @brief Deletes textures from the OpenGL context.
 * 
 * @param tId texture ID to delete in the OpenGL context.
 * @return GfxResult<GLuint> OK if succeeded, FAILURE if error occurred
 */
GfxResult<GLuint> OpenGlEsGfxController::deleteTextures(GLuint *tId) {
    glDeleteTextures(1, tId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::deleteTextures: Error %d\n", error);
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
GfxResult<GLuint> OpenGlEsGfxController::updateBufferData(const vector<GLfloat> &vertices, GLuint vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * vertices.size(), &vertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::updateBufferData: Error %d\n", error);
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
GfxResult<GLuint> OpenGlEsGfxController::setTexParam(TexParam param, TexVal val) {
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
            printf("OpenGlEsGfxController::setTexParam: Unknown parameter option for OpenGL: %d\n",
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
            printf("OpenGlEsGfxController::setTexParam: Unknown parameter option for OpenGL: %d\n",
                static_cast<int>(param));
            break;
    }
    glTexParameteri(GL_TEXTURE_2D, glParam, glVal);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::setTexParam: Error %d\n", error);
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
GfxResult<GLuint> OpenGlEsGfxController::enableVertexAttArray(GLuint layout, size_t size) {
    if (!activeVao_) {
        fprintf(stderr, "OpenGlEsGfxController::enableVertexAttArray: No VAO bound for layout %d\n", layout);
        return GFX_FAILURE(GLuint);
    }
    // Add this attribute call to the VAO bind data
    auto exists = vaoBindData_.find(activeVao_) != vaoBindData_.end();
    if (exists) {
        vaoBindData_[activeVao_][activeVbo_].layout = layout;
        vaoBindData_[activeVao_][activeVbo_].size = size;
    } else {
        fprintf(stderr, "OpenGlEsGfxController::enableVertexAttArray: VAO is not currently bound\n");
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
GfxResult<GLuint> OpenGlEsGfxController::disableVertexAttArray(GLuint layout) {
    glDisableVertexAttribArray(layout);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::disableVertexAttArray: Error %d\n", error);
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
GfxResult<GLuint> OpenGlEsGfxController::drawTriangles(GLuint size) {
    glDrawArrays(GL_TRIANGLES, 0, size);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::drawTriangles: Error %d\n", error);
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
void OpenGlEsGfxController::clear(GfxClearMode clearMode) {
    auto clearVal = 0;
    switch (clearMode) {
        case COLOR:
            clearVal = GL_COLOR_BUFFER_BIT;
            break;
        case DEPTH:
            clearVal = GL_DEPTH_BUFFER_BIT;
            break;
        default:
            printf("OpenGlEsGfxController::clear: Unknown clearMode %d\n", clearMode);
            return;
    }
    glClear(clearVal);
}

/**
 * @brief Deletes a VBO object
 * 
 * @param bufferId pointer to VBO ID of buffer to delete
 */
void OpenGlEsGfxController::deleteBuffer(GLuint *bufferId) {
    glDeleteBuffers(1, bufferId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlEsGfxController::deleteBuffer: Error %d\n", error);
    }
}

/**
 * @brief Deletes a VAO object
 * 
 * @param vao pointer to VAO ID of VAO to delete
 */
void OpenGlEsGfxController::deleteVao(GLuint *vao) {
    // Deletes a vao from the vao list
    auto exists = vaoBindData_.find(*vao) != vaoBindData_.end();
    if (exists) vaoBindData_.erase(*vao);
    *vao = UINT_MAX;
}

