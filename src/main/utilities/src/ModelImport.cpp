/**
 * @file ModelImport.cpp
 * @author Christian Galvez
 * @brief Loads .OBJ files into the app as Polygons that can be rendered in the GameInstance
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <ModelImport.hpp>

ModelImport::ModelImport(string modelPath, vector<string> texturePath, vector<GLint> texturePattern, GLuint programId) :
    modelPath { modelPath }, texturePath { texturePath }, texturePattern { texturePattern }, programId { programId } {
}

Polygon* ModelImport::createPolygonFromFile() {
    ifstream file;  // Read file as read only
    file.open(modelPath);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Model path does not exist!";
        throw runtime_error("Model path does not exist");
    }
    auto polygon = new Polygon();
    auto currentObject = 0;
    textureCount = texturePath.size();
    string charBuffer;  // Will temporarily hold each line in obj file
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
                vertexFrame.push_back(*it);  // Add points to vertexFrame
            }
        } else if (charBuffer.compare(0, 2, "vt") == 0) {
            vector<GLfloat> tempTextures(2);
            sscanf(charBuffer.c_str(), "vt %f %f\n", &tempTextures[0],
                &tempTextures[1]);
            vector<GLfloat>::iterator it;
            for (it = tempTextures.begin(); it != tempTextures.end(); ++it) {
                textureFrame.push_back(*it);  // Add points to textureFrame
            }
        } else if (charBuffer.compare(0, 2, "vn") == 0) {
            vector<GLfloat> tempNormals(3);
            sscanf(charBuffer.c_str(), "vn %f %f %f\n", &tempNormals[0],
                &tempNormals[1], &tempNormals[2]);
            vector<GLfloat>::iterator it;
            for (it = tempNormals.begin(); it != tempNormals.end(); ++it) {
                normalFrame.push_back(*it);  // Add points to normalFrame
            }
        } else if (charBuffer.compare(0, 2, "f ") == 0) {
            vector<GLint> coms(9);
            // If the model is missing texture coordinates, take into account
            if (charBuffer.find("//") != std::string::npos) {
                sscanf(charBuffer.c_str(), "f %i//%i %i//%i %i//%i\n",
                    &coms[0], &coms[2], &coms[3], &coms[5], &coms[6], &coms[8]);
                coms[1] = 0;
                coms[4] = 0;
                coms[7] = 0;
            } else {
                sscanf(charBuffer.c_str(), "f %i/%i/%i %i/%i/%i %i/%i/%i\n",
                    &coms[0], &coms[1], &coms[2], &coms[3], &coms[4], &coms[5],
                    &coms[6], &coms[7], &coms[8]);
            }
            vector<GLint>::iterator it;
            for (it = coms.begin(); it != coms.end(); ++it) {
                commands.push_back(*it);  // Add commands from temp to main vec
            }
        } else if (charBuffer.compare(0, 2, "o ") == 0) {
            if (currentObject) {  // Ignore first object
                // Merge polygon into master polygon object
                polygon->merge(buildObject(currentObject - 1));  // Start index at zero just because
                // Clear frame buffers
                commands.clear();
            }
            ++currentObject;
        }
    }
    // Create the final object in the polygon
    polygon->merge(buildObject(currentObject - 1));
    polygon->textureUniformId = glGetUniformLocation(programId, "mytexture");
    polygon->programId = this->programId;
    return polygon;
}

Polygon* ModelImport::buildObject(int objectId) {
    cout << "Building GameObject " << objectId << endl;
    auto pointCount = commands.size() / 9;
    vector<GLfloat> vertexVbo;
    vector<GLfloat> textureVbo;
    vector<GLfloat> normalVbo;
    /// @todo If any glitches occur, clear frame buffers between each buildObject call...
    // Iterate over each polygon in model
    cout << "pointCount is " << pointCount << endl;
    for (int i = 0; i < pointCount; i++) {
        for (int k = 0; k < 3; k++) {  // Loop through each point
            int currentCommandIndex = (i*9) + (k*3);  // vertex coord command
            int currentCommand = commands[currentCommandIndex];
            // Assign vertex data
            for (int l = 0; l < 3; l++) {
                vertexVbo.push_back(vertexFrame[(currentCommand-1)*3+l]);
            }
            currentCommandIndex = (i*9) + (k*3) + 1;  // textureCoord command
            currentCommand = commands[currentCommandIndex];
            if (textureFrame.size() > 0) {
                textureVbo.push_back(textureFrame[(currentCommand-1)*2]);
                textureVbo.push_back(1.0f - textureFrame[(currentCommand-1)*2+1]);
            } else {
                textureVbo.push_back(0.0f);
                textureVbo.push_back(0.0f);  // Add dummy values for missing data
            }
            currentCommandIndex = (i*9) + (k*3) + 2;  // normal command
            currentCommand = commands[currentCommandIndex];
            for (int l = 0; l < 3; l++) {
                normalVbo.push_back(normalFrame[(currentCommand-1)*3+l]);
            }
        }
    }
    auto polygon = new Polygon(pointCount, this->programId, vertexVbo, textureVbo, normalVbo);
    /// @todo Run configureOpenGL once when all objects are created - figure out deal with destructor
    configureOpenGl(polygon, objectId);
    return polygon;
}

void ModelImport::configureOpenGl(Polygon* polygon, int objectId) {
    glGenBuffers(1, &(polygon->shapeBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon->shapeBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon->pointCount[0] * 9,
                 &(polygon->vertices[0][0]), GL_STATIC_DRAW);
    glGenBuffers(1, &(polygon->normalBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon->normalBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon->pointCount[0] * 9,
                 &(polygon->normalCoords[0][0]), GL_STATIC_DRAW);
    // Specific case where the current object does not get a texture
    if (!textureCount || texturePattern[objectId] >= textureCount ||
        texturePattern[objectId] == -1) {
        return;
    }
    SDL_Surface *texture = IMG_Load(texturePath[texturePattern[objectId]].c_str());
    if (texture == NULL) {
        cerr << "Failed to create SDL_Surface texture!\n";
        return;
    }
    glGenTextures(1, &(polygon->textureId[0]));
    glBindTexture(GL_TEXTURE_2D, polygon->textureId[0]);
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
    SDL_FreeSurface(texture);
    // glGenerateMipmap(GL_TEXTURE_2D);
    glGenBuffers(1, &(polygon->textureCoordsId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon->textureCoordsId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon->pointCount[0] * 6,
                 &(polygon->textureCoords[0][0]), GL_STATIC_DRAW);
}

/**
 * @todo Not sure what to do here yet... but might keep state of OpenGL imports
*/
ModelImport::~ModelImport() {
}
