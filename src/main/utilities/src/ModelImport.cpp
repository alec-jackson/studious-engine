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

ModelImport::ModelImport(string modelPath, vector<string> texturePath, vector<int> texturePattern, unsigned int programId) :
    modelPath_ { modelPath }, texturePath_ { texturePath }, texturePattern_ { texturePattern },
    programId_ { programId } {
    cout << "ModelImport::ModelImport" << endl;
}

/**
 * @brief Attempts to create a Polygon using the .obj file located in modelPath. modelPath is set via the constructor.
 * 
 * @return Polygon* created using .obj file passed into the constructor.
 */
Polygon ModelImport::createPolygonFromFile() {
    // File is closed when ifstream is destroyed
    ifstream file;  // Read file as read only
    file.open(modelPath_);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Model path does not exist!";
        throw runtime_error("Model path does not exist");
    }
    auto currentObject = 0;
    textureCount_ = texturePath_.size();
    string charBuffer;  // Will temporarily hold each line in obj file
    // Grab entire lines from the object file to read at once
    while (getline(file, charBuffer)) {
        currentObject = processLine(charBuffer, currentObject);
    }
    // Create the final object in the polygon
    buildObject(currentObject - 1);
    polygon_.programId = this->programId_;
    polygon_.texturePath_ = texturePath_;
    polygon_.texturePattern_ = texturePattern_;
    return polygon_;
}

/**
 * @brief Processes the current line in the .obj file at modelPath
 * 
 * @param charBuffer A string containing data from the current line in the object file.
 * @param currentObject Index of current object in obj file. Obj files often times contain multiple sub-objects that make up one main object.
 * @param polygon The Polygon that is currently being built
 * @return int containing the current object index - incremented when a new object line is hit in the obj file.
 */
int ModelImport::processLine(string charBuffer, int currentObject) {
    // Compare the first two "header" bytes of the obj file below
    if (charBuffer.compare(0, 2, "v ") == 0) {
        vector<float> tempVertices(3);
        sscanf(charBuffer.c_str(), "v %f %f %f\n", &tempVertices[0],
            &tempVertices[1], &tempVertices[2]);
        // Add tempVertices to vertexFrame
        vector<float>::iterator it;
        for (it = tempVertices.begin(); it != tempVertices.end(); ++it) {
            vertexFrame_.push_back(*it);  // Add points to vertexFrame
        }
    } else if (charBuffer.compare(0, 2, "vt") == 0) {
        vector<float> tempTextures(2);
        sscanf(charBuffer.c_str(), "vt %f %f\n", &tempTextures[0],
            &tempTextures[1]);
        vector<float>::iterator it;
        for (it = tempTextures.begin(); it != tempTextures.end(); ++it) {
            textureFrame_.push_back(*it);  // Add points to textureFrame
        }
    } else if (charBuffer.compare(0, 2, "vn") == 0) {
        vector<float> tempNormals(3);
        sscanf(charBuffer.c_str(), "vn %f %f %f\n", &tempNormals[0],
            &tempNormals[1], &tempNormals[2]);
        vector<float>::iterator it;
        for (it = tempNormals.begin(); it != tempNormals.end(); ++it) {
            normalFrame_.push_back(*it);  // Add points to normalFrame
        }
    } else if (charBuffer.compare(0, 2, "f ") == 0) {
        vector<int> coms(9);
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
        vector<int>::iterator it;
        for (it = coms.begin(); it != coms.end(); ++it) {
            commands_.push_back(*it);  // Add commands from temp to main vec
        }
    } else if (charBuffer.compare(0, 2, "o ") == 0) {
        if (currentObject) {  // Ignore first object
            // Merge polygon into master polygon object
            buildObject(currentObject - 1);  // Start index at zero just because
            // Clear frame buffers
            commands_.clear();
        }
        ++currentObject;
    }
    return currentObject;
}

/**
 * @brief Creates a new Polygon object that can be merged into a parent Polygon object
 * 
 * @param objectId The index of the current object being created relative to other objects in the obj file.
 * @return int 
 * @todo Double check the doxygen comments in here
 */
int ModelImport::buildObject(int objectId) {
    cout << "Building Polygon obj index " << objectId << endl;
    auto triCount = commands_.size() / 9;
    vector<float> vertexVbo;
    vector<float> textureVbo;
    vector<float> normalVbo;
    /// @todo If any glitches occur, clear frame buffers between each buildObject call...
    // Iterate over each polygon in model
    cout << "pointCount is " << triCount << endl;
    for (int i = 0; i < triCount; i++) {
        for (int k = 0; k < 3; k++) {  // Loop through each point
            int currentCommandIndex = (i*9) + (k*3);  // vertex coord command
            int currentCommand = commands_[currentCommandIndex];
            // Assign vertex data
            for (int l = 0; l < 3; l++) {
                vertexVbo.push_back(vertexFrame_[(currentCommand-1)*3+l]);
            }
            currentCommandIndex = (i*9) + (k*3) + 1;  // textureCoord command
            currentCommand = commands_[currentCommandIndex];
            if (textureFrame_.size() > 0) {
                textureVbo.push_back(textureFrame_[(currentCommand-1)*2]);
                textureVbo.push_back(1.0f - textureFrame_[(currentCommand-1)*2+1]);
            } else {
                textureVbo.push_back(0.0f);
                textureVbo.push_back(0.0f);  // Add dummy values for missing data
            }
            currentCommandIndex = (i*9) + (k*3) + 2;  // normal command
            currentCommand = commands_[currentCommandIndex];
            for (int l = 0; l < 3; l++) {
                normalVbo.push_back(normalFrame_[(currentCommand-1)*3+l]);
            }
        }
    }
    auto newPolygon = Polygon(triCount, this->programId_, vertexVbo, textureVbo, normalVbo);
    polygon_.merge(newPolygon);
    return 0;
}

/**
 * @todo Not sure what to do here yet... but might keep state of OpenGL imports
*/
ModelImport::~ModelImport() {
    cout << "ModelImport::~ModelImport" << endl;
}
