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
#include <string>
#include <iostream>
#include <vector>
#include <cstdio>
#include <algorithm>

/**
 * @brief Generates a buffer in the OpenGL context
 *
 * @param bufferId unsigned int to store new buffer ID created in OpenGL context
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::generateBuffer(unsigned int *bufferId) {
#ifdef VERBOSE_LOGS
    printf("OpenGlGfxController::generateBuffer: bufferId %p\n", bufferId);
#endif
    glGenBuffers(1, bufferId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::generateBuffer: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    vboList_.push_back(*bufferId);
    return GFX_OK(unsigned int);
}

/**
 * @brief Binds a buffer to the current OpenGL context
 *
 * @param bufferId ID of buffer to bind (needs to be generated first via generateBuffer)
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::bindBuffer(unsigned int bufferId) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::bindBuffer: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Sends data to the currently bound buffer in the OpenGL context. This transfers data from the application
 * side (c++ studious) to the GPU side (OpenGL in this case).
 *
 * @param size Size of the data array
 * @param data The data array write to the OpenGL buffer
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::sendBufferData(size_t size, void *data) {
#ifdef VERBOSE_LOGS
    printf("OpenGlGfxController::sendBufferData: size %lu data %p\n", size, data);
#endif
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::sendBufferData: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Copies texture data to the currently bound texture buffer.
 *
 * @param width of the texture to send
 * @param height of the texture to send
 * @param format of the texture
 * @param data pixel data to send to the GPU
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::sendTextureData(unsigned int width, unsigned int height, TexFormat format,
    void *data) {
    auto texFormat = GL_RGB;
    switch (format) {
        case TexFormat::RGBA:
            texFormat = GL_RGBA;
            break;
        case TexFormat::RGB:
            texFormat = GL_RGB;
            break;
        case TexFormat::BITMAP:
            texFormat = GL_RED;  // Just need a single color
            break;
        default:
            fprintf(stderr, "OpenGlGfxController::sendTextureData: Unknown texture format %d\n",
                static_cast<std::underlying_type_t<TexFormat>>(format));
            return GFX_FAILURE(unsigned int);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, texFormat, width, height, 0, texFormat, GL_UNSIGNED_BYTE, data);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::sendTextureData: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

GfxResult<uint> OpenGlGfxController::sendTextureData3D(int offsetx, int offsety, int index, uint width, uint height,
    TexFormat format, void *data) {
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,  // Target
        0,  // mipmap level
        offsetx,
        offsety,
        index,
        width,
        height,
        1,  // Just send one layer of data at a time for now...
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::sendTextureData3D: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Generates mipmaps for the currently bound texture
 *
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::generateMipMap() {
    glGenerateMipmap(GL_TEXTURE_2D);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::generateMipMap: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Generates a new texture and writes the new texture ID to the textureId variable passed in.
 *
 * @param textureId assigned to newly created texture in OpenGL context.
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::generateTexture(uint *textureId) {
    glGenTextures(1, textureId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::generateTexture: Error: %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    textureIdList_.push_back(*textureId);
    return GFX_OK(unsigned int);
}

GfxResult<uint> OpenGlGfxController::allocateTexture3D(TexFormat format, uint width, uint height, uint layers) {
    glTexImage3D(GL_TEXTURE_2D_ARRAY,
        0,  // Mipmap level count - not dealing with these for now. -CG
        format == TexFormat::RGB ? GL_RGB8 : GL_RGBA8,  // format
        width,
        height,
        layers,
        0,  // border
        GL_RGBA,  // format
        GL_UNSIGNED_BYTE,  // type
        nullptr);  // data - not required at allocation
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::allocateTexture3D: Error: %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> OpenGlGfxController::getProgramId(string programName) {
    auto result = GFX_FAILURE(uint);
    // Check if the program exists in the program ID map
    auto pimit = programIdMap_.find(programName);
    if (pimit != programIdMap_.end()) {
        result = GfxResult<uint>(GfxApiResult::OK, pimit->second);
    }
    return result;
}

/**
 * @brief Compiles shaders and adds them to the programId list for this GfxController.
 *
 * @param vertexShader Path to the vertexShader to compile on the system
 * @param fragmentShader Path to the fragmentShader to compile on the system
 * @return GfxResult<unsigned int> OK with the newly created programId; FAILURE otherwise.
 */
GfxResult<unsigned int> OpenGlGfxController::loadShaders(string programName, string vertexShader,
    string fragmentShader) {
    unsigned int vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    int logLength;
    int success = GL_FALSE;
    string vertShader, fragShader, tempLine;
    ifstream file;
    file.open(vertexShader);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Error: Cannot open file " << vertexShader << "!\n";
        return GfxResult<unsigned int>(GfxApiResult::FAILURE, UINT_MAX);
    }
    while (getline(file, tempLine)) {
        vertShader.append(tempLine + '\n');
    }
    cout << "Now compiling " << vertexShader << "...\n";
    vector<const char *> vertShaderInfo(1);
    vertShaderInfo[0] = reinterpret_cast<const char *>(vertShader.c_str());
    glShaderSource(vertexShaderID, 1, &vertShaderInfo[0], NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength) {
        vector<char> errorLog(logLength + 1);
        glGetShaderInfoLog(vertexShaderID, logLength, NULL, &errorLog[0]);
        cerr << vertexShader << ": " << &errorLog[0] << "\n";
        assert(false);
    }
    file.close();
    success = GL_FALSE;
    file.open(fragmentShader);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Error: Cannot open file " << fragmentShader << "!\n";
        return GfxResult<unsigned int>(GfxApiResult::FAILURE, UINT_MAX);
    }
    while (getline(file, tempLine)) {
        fragShader.append(tempLine + '\n');
    }
    cout << "Now compiling " << fragmentShader << "...\n";
    vector<const char *>fragShaderInfo(1);
    fragShaderInfo[0] = reinterpret_cast<const char *>(fragShader.c_str());
    glShaderSource(fragmentShaderID, 1, &fragShaderInfo[0], NULL);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength) {
        vector<char> errorLog(logLength + 1);
        glGetShaderInfoLog(fragmentShaderID, logLength, NULL, &errorLog[0]);
        cerr << fragmentShader << ": " << &errorLog[0] << "\n";
        assert(false);
    }
    file.close();
    unsigned int programId = glCreateProgram();
    glAttachShader(programId, vertexShaderID);
    glAttachShader(programId, fragmentShaderID);
    glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength) {
        vector<char> errorLog(logLength + 1);
        glGetProgramInfoLog(programId, logLength, NULL, &errorLog[0]);
        cerr << &errorLog[0] << "\n";
        assert(false);
    }
    glDetachShader(programId, vertexShaderID);
    glDetachShader(programId, fragmentShaderID);
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    // Add programId to programIdMap
    programIdMap_[programName] = programId;

    printf("OpenGlGfxController::loadShaders: Created program %s -> programId %d\n", programName.c_str(), programId);

    return GfxResult<unsigned int>(GfxApiResult::OK, programId);
}

/**
 * @brief Gets the location of a variable in a shader
 *
 * @param programId program to search for variable inside of.
 * @param variableName name of variable to find in shader.
 * @return GfxResult<int> OK with variableId; FAILURE otherwise.
 */
GfxResult<int> OpenGlGfxController::getShaderVariable(unsigned int programId, const char *variableName) {
    auto varId = glGetUniformLocation(programId, variableName);
    auto result = GfxResult<int>(GfxApiResult::OK, varId);
    if (varId == -1) result = GfxResult<int>(GfxApiResult::FAILURE, varId);
    return result;
}

/**
 * @brief Has a list of OpenGL specific commands that update once per frame. This is not part of the GfxController
 * interface.
 *
 */
void OpenGlGfxController::updateOpenGl() {
#ifndef GFX_EMBEDDED
    glEnable(GL_MULTISAMPLE);
#endif  // GFX_EMBEDDED
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(bgColor_[0], bgColor_[1], bgColor_[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::updateOpenGl: Error: %d\n", error);
        return;
    }
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
 * @return GfxResult<int> OK if successful; FAILURE otherwise
 */
GfxResult<int> OpenGlGfxController::init() {
    cout << "OpenGlGfxController::init" << endl;
#ifdef GFX_EMBEDDED
    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress)) {
        cerr << "Error: Failed to initialize GLEW!\n";
        return GFX_FAILURE(int);
    }
#else
if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    cerr << "Error: Failed to initialize GLEW!\n";
    return GFX_FAILURE(int);
}
#endif  // GFX_EMBEDDED
    // Set pixel storage alignment mode for font loading
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return GFX_OK(int);
}

/**
 * @brief Sets the current program (shader) in the OpenGL context.
 *
 * @param programId created by OpenGL for a set of shaders (program).
 * @return GfxResult<unsigned int> OK if successful, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::setProgram(unsigned int programId) {
    glUseProgram(programId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        assert(false);
        fprintf(stderr, "OpenGlGfxController::setProgram: programId %d Error: %d\n", programId, error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Sends a float to a corresponding variable inside of a program (shader)
 *
 * @param variableId of the variable inside of the program to send data to
 * @param data to write to variable
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::sendFloat(unsigned int variableId, float data) {
    glUniform1f(variableId, data);
    return GFX_OK(unsigned int);
}

/**
 * @brief Sends a 3D vector of floats to a shader variable
 *
 * @param variableId variableId to write data to
 * @param count Number of float vectors to send
 * @param data Raw data to send to GPU
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::sendFloatVector(unsigned int variableId, size_t count, float *data) {
    glUniform3fv(variableId, count, data);
    return GFX_OK(unsigned int);
}

/**
 * @brief Sets the rendering mode for triangles.
 * @see RenderMode enum for rendering options.
 *
 * @param mode method for rendering triangles.
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::polygonRenderMode(RenderMode mode) {
    #ifdef GFX_EMBEDDED
    return GFX_OK(unsigned int);
    #else
    auto result = GFX_OK(unsigned int);
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
            fprintf(stderr, "OpenGlGfxController::polygonRenderMode: Unsupported render mode %d",
                static_cast<std::underlying_type_t<RenderMode>>(mode));
            result = GFX_FAILURE(unsigned int);
            break;
    }
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        /// @todo When a logger is added, add OpenGL error log debugging
        fprintf(stderr, "OpenGlGfxController::polygonRenderMode: mode %d, Error: %d\n",
            static_cast<std::underlying_type_t<RenderMode>>(mode), error);
        return GFX_FAILURE(unsigned int);
    }
    return result;
    #endif
}

/**
 * @brief Sends a 4x4 matrix of floats to the GPU memory for a variable in the current program.
 *
 * @param variableId Variable to write data to
 * @param count Number of matrices to write - generally just one
 * @param data Raw data to send over to program variable.
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::sendFloatMatrix(unsigned int variableId, size_t count, float *data) {
    glUniformMatrix4fv(variableId, count, GL_FALSE, data);
    return GFX_OK(unsigned int);
}

/**
 * @brief Sends an integer to the GPU memory for a variable in the current program.
 *
 * @param variableId Variable identifier in the current program
 * @param data Raw data to copy to GPU
 * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
 */
GfxResult<unsigned int> OpenGlGfxController::sendInteger(unsigned int variableId, int data) {
    glUniform1i(variableId, data);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> OpenGlGfxController::bindTexture(uint textureId, GfxTextureType type) {
    GLenum texType;
    switch (type) {
        case GfxTextureType::NORMAL:
            texType = GL_TEXTURE_2D;
            break;
        case GfxTextureType::ARRAY:
            texType = GL_TEXTURE_2D_ARRAY;
            break;
        default:
            texType = GL_TEXTURE_2D;
            break;
    }
    // Use texture unit zero - nothing fancy
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texType, textureId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        /// @todo When a logger is added, add OpenGL error log debugging
        fprintf(stderr, "OpenGlGfxController::bindTexture: textureId %u, Error: %d\n", textureId, error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Binds a VAO object in the current OpenGL context
 * @note The name of this method will most likely change other GFX backends are added.
 *
 * @param vao VAO ID to bind
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::bindVao(unsigned int vao) {
    glBindVertexArray(vao);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        /// @todo When a logger is added, add OpenGL error log debugging
        fprintf(stderr, "OpenGlGfxController::bindVao: vao %u, Error: %d\n", vao, error);
        assert(false);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
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
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::setCapability(GfxCapability capability, bool enabled) {
    auto capabilityId = 0;
    switch (capability) {
        case GfxCapability::CULL_FACE:
            capabilityId = GL_CULL_FACE;
            break;
        default:
            printf("OpenGlGfxController::setCapability: Unknown capability %d\n",
                static_cast<int>(capability));
            return GFX_FAILURE(unsigned int);
    }
    enabled ? glEnable(capabilityId) : glDisable(capabilityId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::setCapability: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Initializes a VAO object
 *
 * @param vao to initialize
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::initVao(unsigned int *vao) {
    glGenVertexArrays(1, vao);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::initVao: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
#ifdef VERBOSE_LOGS
    printf("OpenGlGfxController::initVao: Created vao %d\n", *vao);
#endif
    vaoList_.push_back(*vao);
    return GFX_OK(unsigned int);
}

/**
 * @brief Deletes textures from the OpenGL context.
 *
 * @param tId texture ID to delete in the OpenGL context.
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::deleteTextures(unsigned int *tId) {
    glDeleteTextures(1, tId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::deleteTextures: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    textureIdList_.erase(std::remove(textureIdList_.begin(), textureIdList_.end(), *tId), textureIdList_.end());
    return GFX_OK(unsigned int);
}

/**
 * @brief Updates a VBO buffer with a set of vertices
 *
 * @param vertices Vector of vertex data to copy to VBO object
 * @param vbo identifier for the buffer to write data into
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::updateBufferData(const vector<float> &vertices, unsigned int vbo) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size(), &vertices[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::updateBufferData: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Sets a texture parameter for the texture currently bound in the OpenGL context
 *
 * @param param parameter to update @see TexParam in GfxController
 * @param val value to set for the given parameter
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::setTexParam(TexParam param, TexVal val, GfxTextureType type) {
    // Convert Studious GFX enums to OpenGL enums
    auto glParam = 0;
    auto glVal = 0;
    auto glTexType = 0;
    switch (param) {
        case TexParam::WRAP_MODE_S:
            glParam = GL_TEXTURE_WRAP_S;
            break;
        case TexParam::WRAP_MODE_T:
            glParam = GL_TEXTURE_WRAP_T;
            break;
        case TexParam::MINIFICATION_FILTER:
            glParam = GL_TEXTURE_MIN_FILTER;
            break;
        case TexParam::MAGNIFICATION_FILTER:
            glParam = GL_TEXTURE_MAG_FILTER;
            break;
        case TexParam::MIPMAP_LEVEL:
            glParam = GL_TEXTURE_MAX_LEVEL;
            break;
        default:
            printf("OpenGlGfxController::setTexParam: Unknown parameter option for OpenGL: %d\n",
                static_cast<int>(param));
            return GFX_FAILURE(unsigned int);
    }
    switch (val.type()) {
        case TexValType::CLAMP_TO_EDGE:
            glVal = GL_CLAMP_TO_EDGE;
            break;
        case TexValType::GFX_LINEAR:
            glVal = GL_LINEAR;
            break;
        case TexValType::NEAREST_MIPMAP:
            glVal = GL_NEAREST_MIPMAP_LINEAR;
            break;
        case TexValType::NEAREST_NEIGHBOR:
            glVal = GL_NEAREST;
            break;
        case TexValType::CUSTOM:
            glVal = val.data();
            break;
        default:
            printf("OpenGlGfxController::setTexParam: Unknown parameter option for OpenGL: %d\n",
                static_cast<int>(param));
            break;
    }
    switch (type) {
        case GfxTextureType::NORMAL:
            glTexType = GL_TEXTURE_2D;
            break;
        case GfxTextureType::ARRAY:
            glTexType = GL_TEXTURE_2D_ARRAY;
            break;
        default:
            glTexType = GL_TEXTURE_2D;
            break;
    }
    glTexParameteri(glTexType, glParam, glVal);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::setTexParam: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> OpenGlGfxController::enableVertexAttArray(uint layout, int count, size_t size, void *offset) {
    glVertexAttribPointer(
        layout,                          // layout in shader
        count,                           // size
        GL_FLOAT,                        // type
        GL_FALSE,                        // normalized?
        count * size,                    // stride
        offset);                         // array buffer offset
    glEnableVertexAttribArray(layout);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::enableVertexAttArray: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

GfxResult<uint> OpenGlGfxController::setVertexAttDivisor(uint layout, uint divisor) {
    glVertexAttribDivisor(layout, divisor);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::setVertexAttDivisor: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Disables a vertex attribute array in the OpenGL context.
 *
 * @param layout The layout of the vertex attribute array to disable in the current program.
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::disableVertexAttArray(unsigned int layout) {
    glDisableVertexAttribArray(layout);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::disableVertexAttArray: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

/**
 * @brief Draws triangles using currently bound textures/buffers.
 *
 * @param size The number of vertices to render
 * @return GfxResult<unsigned int> OK if succeeded, FAILURE if error occurred
 */
GfxResult<unsigned int> OpenGlGfxController::drawTriangles(unsigned int size) {
    glDrawArrays(GL_TRIANGLES, 0, size);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::drawTriangles: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
}

GfxResult<uint> OpenGlGfxController::drawTrianglesInstanced(uint size, uint count) {
    glDrawArraysInstanced(GL_TRIANGLES, 0, size, count);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::drawTrianglesInstanced: Error %d\n", error);
        return GFX_FAILURE(unsigned int);
    }
    return GFX_OK(unsigned int);
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
        case GfxClearMode::COLOR:
            clearVal = GL_COLOR_BUFFER_BIT;
            break;
        case GfxClearMode::DEPTH:
            clearVal = GL_DEPTH_BUFFER_BIT;
            break;
        default:
            printf("OpenGlGfxController::clear: Unknown clearMode %d\n",
                static_cast<std::underlying_type_t<GfxClearMode>>(clearMode));
            return;
    }
    glClear(clearVal);
}

/**
 * @brief Deletes a VBO object
 *
 * @param bufferId pointer to VBO ID of buffer to delete
 */
void OpenGlGfxController::deleteBuffer(unsigned int *bufferId) {
    glDeleteBuffers(1, bufferId);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::deleteBuffer: Error %d\n", error);
    } else {
        // Remove the buffer from the vboList
        vboList_.erase(std::remove(vboList_.begin(), vboList_.end(), *bufferId), vboList_.end());
    }
}

/**
 * @brief Deletes a VAO object
 *
 * @param vao pointer to VAO ID of VAO to delete
 */
void OpenGlGfxController::deleteVao(unsigned int *vao) {
    glDeleteVertexArrays(1, vao);
    auto error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGlGfxController::deleteVao: Error %d\n", error);
    } else {
        // Remove the VAO from the vaoList
        vaoList_.erase(std::remove(vaoList_.begin(), vaoList_.end(), *vao), vaoList_.end());
    }
}

OpenGlGfxController::OpenGlGfxController() : bgColor_ { 0.2f, 0.2f, 0.4f } {
    printf("OpenGlGfxController::OpenGlGfxController\n");
}

OpenGlGfxController::~OpenGlGfxController() {
    printf("OpenGlGfxController::~OpenGlGfxController\n");
    /* Delete active VAOs */
    for (auto vao : vaoList_) {
        glDeleteVertexArrays(1, &vao);
    }
    /* Delete active VBOs */
    for (auto vbo : vboList_) {
        glDeleteBuffers(1, &vbo);
    }
    /* Delete textures */
    for (auto textureId : textureIdList_) {
        glDeleteTextures(1, &textureId);
    }
    /* Delete Shader Programs */
    for (auto programEntry : programIdMap_) {
        glDeleteProgram(programEntry.second);
    }

    vaoList_.clear();
    vboList_.clear();
    textureIdList_.clear();
    programIdMap_.clear();
}

void OpenGlGfxController::setBgColor(float r, float g, float b) {
    bgColor_ = { r, g, b };
}
