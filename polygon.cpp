#include "polygon.hpp"

Polygon::Polygon(ImportObjInfo &importObjInfo) {
    ifstream file; // Read file as read only
    file.open(importObjInfo.modelPath);
    if (!file.is_open()) { // If the file does not exist or cannot be opened
        //string s = std::format("Model {} not found!", importObjInfo.modelPath);
        throw std::runtime_error("Model not found!");
    }
    int currentObject = 0;
    string charBuffer; // Will temporarily hold each line in obj file
    programID = importObjInfo.programID; // Save the current programID to use
    vector<GLfloat> vertexFrame; // Unique vertex points
    vector<GLfloat> normalFrame; // Unique normal points
    vector<GLfloat> textureFrame; // Unique texture points
    vector<GLint> commands; // Commands in obj file to build buffers
    // Grab entire lines from the object file to read at once
    while (getline(file, charBuffer)) {
        // Compare the first two "header" bytes of the obj file below
        if (charBuffer.compare(0, 2, "v ") == 0) {
            vector<GLfloat> tempVertices(3);
            sscanf(charBuffer.c_str(), "v %f %f %f\n", &tempVertices[0],
                &tempVertices[1], &tempVertices[2]);
            // Add tempVertices to vertexFrame
            vector<GLfloat>::iterator it;
            for (it = tempVertices.begin(); it != tempVertices.end(); ++it) {
                vertexFrame.push_back(*it); // Add points to vertexFrame
            }
        } else if (charBuffer.compare(0, 2, "vt") == 0) {
            vector<GLfloat> tempTextures(2);
            sscanf(charBuffer.c_str(), "vt %f %f\n", &tempTextures[0],
                &tempTextures[1]);
            vector<GLfloat>::iterator it;
            for (it = tempTextures.begin(); it != tempTextures.end(); ++it) {
                textureFrame.push_back(*it); // Add points to textureFrame
            }
        } else if (charBuffer.compare(0, 2, "vn") == 0) {
            vector<GLfloat> tempNormals(3);
            sscanf(charBuffer.c_str(), "vn %f %f %f\n", &tempNormals[0],
                &tempNormals[1], &tempNormals[2]);
            vector<GLfloat>::iterator it;
            for (it = tempNormals.begin(); it != tempNormals.end(); ++it) {
                normalFrame.push_back(*it); // Add points to normalFrame
            }
        } else if (charBuffer.compare(0, 2, "f ") == 0) {
            vector<GLint> coms(9);
            // If the model is missing texture coordinates, take into account
            if (charBuffer.find("//") != std::string::npos) {
                sscanf(charBuffer.c_str(), "f %i//%i %i//%i %i//%i\n",
                    &coms[0], &coms[2], &coms[3], &coms[5], &coms[6], &coms[8]);
                coms[1] = 0; coms[4] = 0; coms[7] = 0;
            } else {
                sscanf(charBuffer.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
                    &coms[0], &coms[1], &coms[2], &coms[3], &coms[4], &coms[5],
                    &coms[6], &coms[7], &coms[8]);
            }
            vector<GLint>::iterator it;
            for (it = coms.begin(); it != coms.end(); ++it) {
                commands.push_back(*it); // Add commands from temp to main vec
            }
            // When we hit the first obect, ignore
        } else if (charBuffer.compare(0, 2, "o ") == 0) {
            // When currentObject == 0, run the following
            if (!currentObject) {
                currentObject++;
            } else {
                configureObject({ vertexFrame, textureFrame, normalFrame, commands, currentObject - 1,
                    importObjInfo.texturePath.size(), importObjInfo.texturePattern, importObjInfo.texturePath });
                currentObject++;
            }
        }
    }
    cout << "Built frames\n";
    // Send the final object to be configured in the function.
    configureObject({ vertexFrame, textureFrame, normalFrame, commands, currentObject - 1,
                    importObjInfo.texturePath.size(), importObjInfo.texturePattern, importObjInfo.texturePath });
    textureUniformID = glGetUniformLocation(importObjInfo.programID,
        "mytexture");
    numberOfObjects = currentObject;
}

Polygon::Polygon(GLuint shapebufferId, GLuint textureCoordsId, GLuint textureId, 
    glfVec vertices, GLint pointCount, GLuint programId) {
        this->shapebufferID.push_back(shapebufferId);
        this->textureCoordsID.push_back(textureCoordsId);
        this->textureID.push_back(textureId);
        this->vertices.push_back(vertices);
        this->pointCount.push_back(pointCount);
        this->programID = programId;
}

void Polygon::configureObject(const ConfigureArgs &configureArgs) {
    cout << "Configuring Object\n";
    pointCount.push_back(configureArgs.commands.size() / 9);
    vector<GLfloat> vertexVBO;
    vector<GLfloat> textureVBO;
    vector<GLfloat> normalVBO;
    // Iterate over each polygon in model
    for (int i = 0; i < pointCount[configureArgs.index]; i++) {
        for (int k = 0; k < 3; k++) { // Loop through each point
            int currentCommandIndex = (i*9) + (k*3); // vertex coord command
            int currentCommand = configureArgs.commands[currentCommandIndex];
            // Assign vertex data
            for (int l = 0; l < 3; l++) {
                vertexVBO.push_back(configureArgs.vertexFrame[(currentCommand-1)*3+l]);
            }
            currentCommandIndex = (i*9) + (k*3) + 1; // textureCoord command
            currentCommand = configureArgs.commands[currentCommandIndex];
            if (configureArgs.textureFrame.size() > 0) {
                textureVBO.push_back(configureArgs.textureFrame[(currentCommand-1)*2]);
                textureVBO.push_back(1.0f - configureArgs.textureFrame[(currentCommand-1)*2+1]);
            } else {
                textureVBO.push_back(0.0f);
                textureVBO.push_back(0.0f); // Add dummy values for missing data
            }
            currentCommandIndex = (i*9) + (k*3) + 2; // normal command
            currentCommand = configureArgs.commands[currentCommandIndex];
            for (int l = 0; l < 3; l++) {
                normalVBO.push_back(configureArgs.normalFrame[(currentCommand-1)*3+l]);
            }
        }
    }
    vertices.push_back(vertexVBO);
    textureCoords.push_back(textureVBO);
    normalCoords.push_back(normalVBO);
    shapebufferID.push_back(0); // Create space on vector for buffer
    normalbufferID.push_back(0); // Create space for normal buff
    // Texture values will default to UINT_MAX to signify no texture
    textureID.push_back(UINT_MAX);
    textureCoordsID.push_back(UINT_MAX);
    glGenBuffers(1, &(shapebufferID[configureArgs.index]));
    glBindBuffer(GL_ARRAY_BUFFER, shapebufferID[configureArgs.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * pointCount[configureArgs.index] * 9,
                 &(vertices[configureArgs.index][0]), GL_STATIC_DRAW);
    glGenBuffers(1, &(normalbufferID[configureArgs.index]));
    glBindBuffer(GL_ARRAY_BUFFER, normalbufferID[configureArgs.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * pointCount[configureArgs.index] * 9,
                 &(normalCoords[configureArgs.index][0]), GL_STATIC_DRAW);
    // Specific case where the current object does not get a texture
    if (!configureArgs.textureCount || configureArgs.texturePattern[configureArgs.index] >= configureArgs.textureCount || configureArgs.texturePattern[configureArgs.index] == -1) {
        return;
    }
    SDL_Surface *texture = IMG_Load(configureArgs.texturePath[configureArgs.texturePattern[configureArgs.index]].c_str());
    if (texture == NULL) {
        cerr << "Failed to create SDL_Surface texture!\n";
        return;
    }
    glGenTextures(1, &(textureID[configureArgs.index]));
    glBindTexture(GL_TEXTURE_2D, textureID[configureArgs.index]);
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
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
    glGenerateMipmap(GL_TEXTURE_2D);
    SDL_FreeSurface(texture);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glGenBuffers(1, &(textureCoordsID[configureArgs.index]));
    glBindBuffer(GL_ARRAY_BUFFER, textureCoordsID[configureArgs.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * pointCount[configureArgs.index] * 6,
                 &(textureCoords[configureArgs.index][0]), GL_STATIC_DRAW);
}

GLint Polygon::getNumberOfObjects() const {
    return numberOfObjects;
}

GLuint Polygon::getProgramId() const {
    return programID;
}

GLuint Polygon::getTextureUniformId() const {
    return textureUniformID;
}

GLuint *Polygon::getShapeBufferIdAddr(unsigned int index) {
    if (index > shapebufferID.size()) throw std::runtime_error("Index out of bounds error.");
    return &shapebufferID[index];
}

GLuint Polygon::getShapeBufferId(unsigned int index) const {
    if (index > shapebufferID.size()) throw std::runtime_error("Index out of bounds error.");
    return shapebufferID[index];
}

GLuint Polygon::getTextureCoordsId(unsigned int index) const {
    if (index > textureCoordsID.size()) throw std::runtime_error("Index out of bounds error.");
    return textureCoordsID.at(index);
}

GLuint Polygon::getNormalBufferId(unsigned int index) const {
    if (index > normalbufferID.size()) throw std::runtime_error("Index out of bounds error.");
    return normalbufferID.at(index);
}

GLuint Polygon::getTextureId(unsigned int index) const {
    if (index > textureID.size()) throw std::runtime_error("Index out of bounds error.");
    return textureID.at(index);
}

GLint Polygon::getPointCount(unsigned int index) const {
    if (index > pointCount.size()) throw std::runtime_error("Index out of bounds error.");
    return pointCount[index];
}

glfVVec Polygon::getVertices() const {
    return vertices;
}

GLfloat *Polygon::getVerticiesLocation(int x, int y) {
    return &vertices[x][y];
}
