#include "ModelImport.hpp"

/**
 * @brief Constructs the ModelImport object using the importObjInfo as parameters
 * @param objInfo that contains the .obj file path, texture path, texture pattern and shader program ID
 * @throws runtime_error exception when passed a model path that does not exist
*/
ModelImport::ModelImport(importObjInfo objInfo) {
    ifstream file; // Read file as read only
    file.open(objInfo.modelPath);
    if (!file.is_open()) { // If the file does not exist or cannot be opened
        cerr << "Model path does not exist!";
        throw runtime_error("Model path does not exist");
    }
    auto currentObject = 0;
    string charBuffer; // Will temporarily hold each line in obj file
    model.programID = objInfo.programID; // Save the current programID to use
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
                configureArgs args;
                args.vertexFrame = vertexFrame;
                args.textureFrame = textureFrame;
                args.normalFrame = normalFrame;
                args.commands = commands;
                args.index = currentObject - 1;
                args.textureCount = objInfo.texturePath.size();
                args.texturePattern = objInfo.texturePattern;
                args.texturePath = objInfo.texturePath;
                configureObject(args);
                currentObject++;
            }
        }
    }
    cout << "Built frames\n";
    // Send the final object to be configured in the function.
    configureArgs args;
    args.vertexFrame = vertexFrame;
    args.textureFrame = textureFrame;
    args.normalFrame = normalFrame;
    args.commands = commands;
    args.index = currentObject - 1;
    args.textureCount = objInfo.texturePath.size();
    args.texturePattern = objInfo.texturePattern;
    args.texturePath = objInfo.texturePath;
    configureObject(args);
    model.textureUniformID = glGetUniformLocation(objInfo.programID,
        "mytexture");
    model.numberOfObjects = currentObject;
}

/**
 * @brief Builds objects in the model using the passed vertex, texture and normal frames; also configures OpenGL related settings for the object
 * @param args containing details for current object @see configureArgs
 * @bug This method and the constructor need to be refactored, this single method should not need 10 arguments
*/
void ModelImport::configureObject(configureArgs args) {
    cout << "Configuring Object\n";
    model.pointCount.push_back(args.commands.size() / 9);
    vector<GLfloat> vertexVBO;
    vector<GLfloat> textureVBO;
    vector<GLfloat> normalVBO;
    // Iterate over each polygon in model
    for (int i = 0; i < model.pointCount[args.index]; i++) {
        for (int k = 0; k < 3; k++) { // Loop through each point
            int currentCommandIndex = (i*9) + (k*3); // vertex coord command
            int currentCommand = args.commands[currentCommandIndex];
            // Assign vertex data
            for (int l = 0; l < 3; l++) {
                vertexVBO.push_back(args.vertexFrame[(currentCommand-1)*3+l]);
            }
            currentCommandIndex = (i*9) + (k*3) + 1; // textureCoord command
            currentCommand = args.commands[currentCommandIndex];
            if (args.textureFrame.size() > 0) {
                textureVBO.push_back(args.textureFrame[(currentCommand-1)*2]);
                textureVBO.push_back(1.0f - args.textureFrame[(currentCommand-1)*2+1]);
            } else {
                textureVBO.push_back(0.0f);
                textureVBO.push_back(0.0f); // Add dummy values for missing data
            }
            currentCommandIndex = (i*9) + (k*3) + 2; // normal command
            currentCommand = args.commands[currentCommandIndex];
            for (int l = 0; l < 3; l++) {
                normalVBO.push_back(args.normalFrame[(currentCommand-1)*3+l]);
            }
        }
    }
    model.vertices.push_back(vertexVBO);
    model.textureCoords.push_back(textureVBO);
    model.normalCoords.push_back(normalVBO);
    model.shapebufferID.push_back(0); // Create space on vector for buffer
    model.normalbufferID.push_back(0); // Create space for normal buff
    // Texture values will default to UINT_MAX to signify no texture
    model.textureID.push_back(UINT_MAX);
    model.textureCoordsID.push_back(UINT_MAX);
    glGenBuffers(1, &(model.shapebufferID[args.index]));
    glBindBuffer(GL_ARRAY_BUFFER, model.shapebufferID[args.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * model.pointCount[args.index] * 9,
                 &(model.vertices[args.index][0]), GL_STATIC_DRAW);
    glGenBuffers(1, &(model.normalbufferID[args.index]));
    glBindBuffer(GL_ARRAY_BUFFER, model.normalbufferID[args.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * model.pointCount[args.index] * 9,
                 &(model.normalCoords[args.index][0]), GL_STATIC_DRAW);
    // Specific case where the current object does not get a texture
    if (!args.textureCount || args.texturePattern[args.index] >= args.textureCount || args.texturePattern[args.index] == -1) {
        return;
    }
    SDL_Surface *texture = IMG_Load(args.texturePath[args.texturePattern[args.index]].c_str());
    if (texture == NULL) {
        cerr << "Failed to create SDL_Surface texture!\n";
        return;
    }
    glGenTextures(1, &(model.textureID[args.index]));
    glBindTexture(GL_TEXTURE_2D, model.textureID[args.index]);
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
    glGenBuffers(1, &(model.textureCoordsID[args.index]));
    glBindBuffer(GL_ARRAY_BUFFER, model.textureCoordsID[args.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * model.pointCount[args.index] * 6,
                 &(model.textureCoords[args.index][0]), GL_STATIC_DRAW);
}

/**
 * @brief Cleans up memory allocated by OpenGL for the imported object
*/
ModelImport::~ModelImport() {
    // Delete OpenGL buffers for the gameObjects
    for (int i = 0; i < model.numberOfObjects; i++) {
        glDeleteBuffers(1, &model.shapebufferID[i]);
        glDeleteBuffers(1, &model.textureCoordsID[i]);
        glDeleteBuffers(1, &model.normalbufferID[i]);
        glDeleteTextures(1, &model.textureID[i]);
    }
}

/**
 * @brief Returns a pointer to the model member
 * @return polygon * pointing to internal polygon member
*/
polygon *ModelImport::getPolygon() {
    return &model;
}