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

/// @todo Add proper error checking to OpenGL calls
GfxResult<GLint> OpenGlGfxController::generateVertexBuffer(Polygon &polygon) {
    cout << "OpenGlGfxController::generateVertexBuffer" << endl;
    glGenBuffers(1, &(polygon.shapeBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon.shapeBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon.pointCount[0] * 9,
                 &(polygon.vertices[0][0]), GL_STATIC_DRAW);
    /// @todo? Error check
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLint> OpenGlGfxController::generateNormalBuffer(Polygon &polygon) {
    cout << "OpenGlGfxController::generateNormalBuffer" << endl;
    glGenBuffers(1, &(polygon.normalBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon.normalBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon.pointCount[0] * 9,
                 &(polygon.normalCoords[0][0]), GL_STATIC_DRAW);
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLint> OpenGlGfxController::generateTextureBuffer(Polygon &polygon, SDL_Surface *texture) {
    cout << "OpenGlGfxController::generateTextureBuffer" << endl;
    if (texture == nullptr) return GfxResult<GLint>(GfxApiResult::FAILURE, -1);
    glGenTextures(1, &(polygon.textureId[0]));
    glBindTexture(GL_TEXTURE_2D, polygon.textureId[0]);
    if (texture->format->Amask) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->w, texture->h, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, texture->pixels);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
    glGenerateMipmap(GL_TEXTURE_2D);

    // glGenerateMipmap(GL_TEXTURE_2D);
    glGenBuffers(1, &(polygon.textureCoordsId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon.textureCoordsId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon.pointCount[0] * 6,
                 &(polygon.textureCoords[0][0]), GL_STATIC_DRAW);

    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLint> OpenGlGfxController::cleanup() {
    auto deletedPrograms = 0;
    for (auto it = programIdList_.begin(); it != programIdList_.end(); ++it) {
        glDeleteProgram(*it);
        deletedPrograms++;
    }
    glDeleteVertexArrays(1, &vertexArrayId_);
    return GfxResult<GLint>(GfxApiResult::OK, deletedPrograms);
}

GfxResult<GLuint> OpenGlGfxController::getProgramId(uint index) {
    // Boundary check index
    if (index > programIdList_.size()) {
        return GfxResult<GLuint>(GfxApiResult::FAILURE, UINT_MAX);
    }
    return GfxResult<GLuint>(GfxApiResult::OK, programIdList_[index]);
}

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

GfxResult<GLint> OpenGlGfxController::getShaderVariable(GLuint programId, const char *variableName) {
    auto varId = glGetUniformLocation(programId, variableName);
    auto result = GfxResult<GLint>(GfxApiResult::FAILURE, varId);
    if (varId == -1) result = GfxResult<GLint>(GfxApiResult::OK, varId);
    return result;
}

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

void OpenGlGfxController::update() {
    glBindVertexArray(vertexArrayId_);
    updateOpenGl();
}

GfxResult<GLint> OpenGlGfxController::init() {
    cout << "OpenGlGfxController::init" << endl;
    glGenVertexArrays(1, &vertexArrayId_);
    glBindVertexArray(vertexArrayId_);
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLuint> OpenGlGfxController::setProgram(GLuint programId) {
    glUseProgram(programId);
    return GfxResult<GLuint>(GfxApiResult::OK, 0);
}

GfxResult<GLuint> OpenGlGfxController::sendFloat(GLuint variableId, GLfloat data) {
    glUniform1f(variableId, data);
    return GfxResult<GLuint>(GfxApiResult::OK, 0);
}

GfxResult<GLuint> OpenGlGfxController::sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) {
    glUniform3fv(variableId, count, data);
    return GFX_OK(GLuint);
}

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

GfxResult<GLuint> OpenGlGfxController::sendFloatMatrix(GLuint variableId, GLsizei count, GLfloat *data) {
    glUniformMatrix4fv(variableId, count, GL_FALSE, data);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> OpenGlGfxController::sendInteger(GLuint variableId, GLint data) {
    glUniform1i(variableId, data);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> OpenGlGfxController::bindTexture(GLuint textureId, GLuint samplerId) {
    // Use texture unit zero - nothing fancy
    glActiveTexture(GL_TEXTURE0);
    // Binds the specific textureId to a GL_TEXTURE_2D - might only need to do once?
    glBindTexture(GL_TEXTURE_2D, textureId);
    // textureUniformId points to the sampler2D in GLSL, point it to texture unit 0
    sendInteger(samplerId, 0);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> OpenGlGfxController::render(GLuint vId, GLuint tId, GLuint nId, GLuint vertexCount) {
    // If textureId is UINT_MAX, NO TEXTURE PRESENT
    // Bind the VAO
    glBindVertexArray(vertexArrayId_);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vId);
    glVertexAttribPointer(
                            0,            // attribute 0. No particular reason for 0, but must match
                                          // the layout (location) in the shader.
                            3,            // size
                            GL_FLOAT,     // type
                            GL_FALSE,     // normalized?
                            0,            // stride
                            0);           // array buffer offset
    glBindBuffer(GL_ARRAY_BUFFER, nId);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    if (tId != UINT_MAX) {
        glBindBuffer(GL_ARRAY_BUFFER, tId);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
    return GFX_OK(GLuint);
}