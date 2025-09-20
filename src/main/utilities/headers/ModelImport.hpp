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
#include <utility>
#include <memory>
#include <Polygon.hpp>
#include <winsup.hpp>
#define DEFAULT_VECTOR_SIZE 256

using std::ifstream;

/**
 * @author Christian Galvez
 * @date 05/06/23
 * @brief Class used for importing .obj files for use in studious engine.
*/
class ModelImport {
 public:
      explicit ModelImport(string modelPath, string materialPath);
      std::shared_ptr<Polygon> createPolygonFromFile();
      void processMaterialFile();
      // Using the move constructor so we don't need to define a copy constructor
      inline std::shared_ptr<Polygon> getPolygon() { return polygon_; }
      int processLine(string, int);
      int buildObject(int objectId);
      ~ModelImport();
 private:
      string modelPath_;
      string materialPath_;
      vector<string> texturePath_;
      vector<int> texturePattern_;
      int textureCount_;  // Size of the texturePath vector
      vector<float> vertexFrame_;  // Unique vertex points
      vector<float> normalFrame_;  // Unique normal points
      vector<float> textureFrame_;  // Unique texture points
      vector<int> commands_;  // Commands in obj file to build buffers
      std::shared_ptr<Polygon> polygon_;
      queue<string> objectNames;
      string matName;
};
