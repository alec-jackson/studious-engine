/**
 * @file ModelImport.hpp
 * @author Christian Galvez
 * @brief Contains ModelImport class definition and structs for ModelImport
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <string>
#include <vector>
#include <Polygon.hpp>

using std::ifstream;

/**
 * @author Christian Galvez
 * @date 05/06/23
 * @brief Class used for importing .obj files for use in studious engine. 
*/
class ModelImport {
 public:
      explicit ModelImport(string, vector<string>, vector<GLint>, GLuint);
      Polygon *createPolygonFromFile();
      int processLine(string, int, Polygon *, bool);
      Polygon *buildObject(int objectId, bool oglConfigure);
      void configureOpenGl(Polygon *, int);
      ~ModelImport();
 private:
      string modelPath;
      vector<string> texturePath;
      vector<GLint> texturePattern;
      int textureCount;  // Size of the texturePath vector
      GLuint programId;
      vector<GLfloat> vertexFrame;  // Unique vertex points
      vector<GLfloat> normalFrame;  // Unique normal points
      vector<GLfloat> textureFrame;  // Unique texture points
      vector<GLint> commands;  // Commands in obj file to build buffers
};
