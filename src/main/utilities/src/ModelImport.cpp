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

// Include External Libraries
#include <cstdio>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <memory>

// Include Internal Headers
#include <ModelImport.hpp>
namespace ModelImport {
Result processObjectFile(string modelPath, std::shared_ptr<Polygon> polygon);
Result processMaterialFile(string modelPath, std::shared_ptr<Polygon> polygon);
std::shared_ptr<Model> buildModel(string matName, const vector<float> &vF, const vector<float> &tF,
    const vector<float> &nF, const vector<int> &commands);
/**
    * @brief Attempts to create a Polygon using the .obj file located in modelPath. modelPath is set via the constructor.
    *
    * @return Polygon* created using .obj file passed into the constructor.
    */
std::shared_ptr<Polygon> createPolygonFromFile(string modelPath) {
    auto polygon = std::make_shared<Polygon>();
    processObjectFile(modelPath, polygon);
    // Re-think data encapsulation - what do we want from each function call?
    // Read mat
    processMaterialFile(modelPath, polygon);
    return polygon;
}

Result processMaterialFile(string modelPath, std::shared_ptr<Polygon> polygon) {
    // Find the material path
    // Check if this works on Windows later
    auto lastPathDelim = modelPath.find_last_of('/');
    string objectDirectory;
    if (std::string::npos != lastPathDelim) {
        objectDirectory = modelPath.substr(0, lastPathDelim) + "/";
    }
    string materialPath = objectDirectory + polygon.get()->materialLibrary;
    // Read the material file
    ifstream file;
    file.open(materialPath);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Material path does not exist!" << materialPath << endl;
        return Result::FAILURE;  // No material processing is done
    }
    string charBuffer;
    string currentMaterial;
    char miscbuffer[MAX_MAT_NAME_SIZE];
    while (getline(file, charBuffer)) {
        if (charBuffer.size() > 0 && charBuffer.front() == '#') {
            continue;  // Ignore comment lines
        }
        if (charBuffer.compare(0, 6, "newmtl") == 0) {
            // This is the material definition line, configure as current material
            assert(sscanf(charBuffer.c_str(), "newmtl %s\n", miscbuffer) != EOF);
            fprintf(stderr, "Creating new material %s\n", miscbuffer);
            currentMaterial = miscbuffer;  // Set current material
            polygon.get()->materialMap[currentMaterial] = std::make_shared<Material>();
            polygon.get()->materialMap[currentMaterial].get()->name = std::string(miscbuffer);
        } else if (charBuffer.compare(0, 2, "Ns") == 0) {
            sscanf(charBuffer.c_str(), "Ns %f\n", &polygon.get()->materialMap[currentMaterial].get()->Ns);
        } else if (charBuffer.compare(0, 6, "map_Kd") == 0) {
            sscanf(charBuffer.c_str(), "map_Kd %s\n", miscbuffer);
            polygon.get()->materialMap[currentMaterial].get()->map_Kd = miscbuffer;
            polygon.get()->materialMap[currentMaterial].get()->pathToTextureFile = objectDirectory + std::string(miscbuffer);
        }
    }
    return Result::OK;
}

/**
    * @brief Processes the current line in the .obj file at modelPath
    *
    * @param charBuffer A string containing data from the current line in the object file.
    * @param currentObject Index of current object in obj file. Obj files often times contain multiple sub-objects that make up one main object.
    * @param polygon The Polygon that is currently being built
    * @return int containing the current object index - incremented when a new object line is hit in the obj file.
    */
Result processObjectFile(string modelPath, std::shared_ptr<Polygon> polygon) {
    ifstream file;  // Read file as read only
    file.open(modelPath);
    if (!file.is_open()) {  // If the file does not exist or cannot be opened
        cerr << "Model path does not exist!";
        return Result::FAILURE;
    }
    string charBuffer;
    vector<float> vertexFrame;
    vector<float> textureFrame;
    vector<float> normalFrame;
    vector<int> commands;
    char miscbuffer[MAX_MAT_NAME_SIZE];
    string matName;
    string prevObjectName;
    while (getline(file, charBuffer)) {
        // Compare the first two "header" bytes of the obj file below
        if (charBuffer.compare(0, 2, "v ") == 0) {
            vector<float> tempVertices(3);
            sscanf(charBuffer.c_str(), "v %f %f %f\n", &tempVertices[0],
                &tempVertices[1], &tempVertices[2]);
            // Add tempVertices to vertexFrame
            for (auto vertex : tempVertices) {
                vertexFrame.push_back(vertex);  // Add points to vertexFrame
            }
        } else if (charBuffer.compare(0, 2, "vt") == 0) {
            vector<float> tempTextures(2);
            sscanf(charBuffer.c_str(), "vt %f %f\n", &tempTextures[0],
                &tempTextures[1]);
            for (auto texCoord : tempTextures) {
                textureFrame.push_back(texCoord);  // Add points to textureFrame
            }
        } else if (charBuffer.compare(0, 2, "vn") == 0) {
            vector<float> tempNormals(3);
            sscanf(charBuffer.c_str(), "vn %f %f %f\n", &tempNormals[0],
                &tempNormals[1], &tempNormals[2]);
            for (auto normal : tempNormals) {
                normalFrame.push_back(normal);  // Add points to normalFrame
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
            for (auto command : coms) {
                commands.push_back(command);  // Add commands from temp to main vec
            }
        } else if (charBuffer.compare(0, 2, "o ") == 0) {
            memset(miscbuffer, 0, sizeof(miscbuffer));
            sscanf(charBuffer.c_str(), "o %s\n", miscbuffer);
            string objectName(miscbuffer);
            if (!prevObjectName.empty()) {  // Ignore first object
                // Merge polygon into master polygon object
                auto model = buildModel(matName, vertexFrame,
                    textureFrame, normalFrame, commands);
                // Clear active commands
                commands.clear();
                // Add the model to the polygon
                polygon.get()->modelMap[prevObjectName] = model;
            }
            prevObjectName = objectName;
        } else if (charBuffer.size() > 7 && charBuffer.compare(0, 7, "usemtl ") == 0) {
            memset(miscbuffer, 0, sizeof(miscbuffer));
            sscanf(charBuffer.c_str(), "usemtl %s\n", miscbuffer);
            string materialName(miscbuffer);
            matName = materialName;
        } else if (charBuffer.size() > 7 && charBuffer.compare(0, 7, "mtllib ") == 0) {
            memset(miscbuffer, 0, sizeof(miscbuffer));
            sscanf(charBuffer.c_str(), "mtllib %s\n", miscbuffer);
            string matlib(miscbuffer);
            polygon.get()->materialLibrary = matlib;
        } else {
            fprintf(stderr, "ModelImport::processLine: Discarding line %s\n",
                charBuffer.c_str());
        }
    }
    file.close();
    // Construct the final object, since we construct in a bottom-up fashion
    if (!prevObjectName.empty()) {
        // Merge polygon into master polygon object
        auto model = buildModel(matName, vertexFrame,
            textureFrame, normalFrame, commands);
        // Clear active commands
        commands.clear();
        // Add the model to the polygon
        polygon.get()->modelMap[prevObjectName] = model;
    }
    return Result::OK;
}

std::shared_ptr<Model> buildModel(string matName, const vector<float> &vF, const vector<float> &tF,
    const vector<float> &nF, const vector<int> &commands) {
    uint triCount = commands.size() / 9;
    vector<float> vertexVbo;
    vector<float> textureVbo;
    vector<float> normalVbo;
    /// @todo If any glitches occur, clear frame buffers between each buildModel call...
    // Iterate over each polygon in model
    cout << "pointCount is " << triCount << endl;
    for (uint i = 0; i < triCount; i++) {
        for (uint k = 0; k < 3; k++) {  // Loop through each point
            uint currentCommandIndex = (i*9) + (k*3);  // vertex coord command
            uint currentCommand = commands[currentCommandIndex];
            // Assign vertex data
            for (uint l = 0; l < 3; l++) {
                vertexVbo.push_back(vF[(currentCommand-1)*3+l]);
            }
            currentCommandIndex = (i*9) + (k*3) + 1;  // textureCoord command
            currentCommand = commands[currentCommandIndex];
            if (tF.size() > 0) {
                textureVbo.push_back(tF[(currentCommand-1)*2]);
                textureVbo.push_back(1.0f - tF[(currentCommand-1)*2+1]);
            } else {
                textureVbo.push_back(0.0f);
                textureVbo.push_back(0.0f);  // Add dummy values for missing data
            }
            currentCommandIndex = (i*9) + (k*3) + 2;  // normal command
            currentCommand = commands[currentCommandIndex];
            for (uint l = 0; l < 3; l++) {
                normalVbo.push_back(nF[(currentCommand-1)*3+l]);
            }
        }
    }
    auto newModel = std::make_shared<Model>(triCount, vertexVbo, textureVbo, normalVbo);
    newModel.get()->materialName = matName;
    return newModel;
}

}
