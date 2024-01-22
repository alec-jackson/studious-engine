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
#include <GfxController.hpp>
#define DEFAULT_VECTOR_SIZE 256

using std::ifstream;

/**
 * @author Christian Galvez
 * @date 05/06/23
 * @brief Class used for importing .obj files for use in studious engine. 
*/
class ModelImport {
 public:
      explicit ModelImport(string, vector<string>, vector<GLint>, GLuint, GfxController &);
      Polygon createPolygonFromFile();
      inline Polygon getPolygon() { return polygon_; }
      int processLine(string, int);
      int buildObject(int objectId);
      void configureOpenGl(Polygon &, int);
      ~ModelImport();
 private:
      string modelPath_;
      vector<string> texturePath_;
      vector<GLint> texturePattern_;
      int textureCount_;  // Size of the texturePath vector
      GLuint programId_;
      vector<GLfloat> vertexFrame_;  // Unique vertex points
      vector<GLfloat> normalFrame_;  // Unique normal points
      vector<GLfloat> textureFrame_;  // Unique texture points
      vector<GLint> commands_;  // Commands in obj file to build buffers
      Polygon polygon_;
      GfxController &gfxController_;
};
