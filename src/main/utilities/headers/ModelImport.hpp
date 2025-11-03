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
#define MAX_MAT_NAME_SIZE 64

using std::ifstream;

/**
 * @author Christian Galvez
 * @date 05/06/23
 * @brief Class used for importing .obj files for use in studious engine.
*/
namespace ModelImport {
enum class Result {
    OK,
    FAILURE
};
std::shared_ptr<Polygon> createPolygonFromFile(string modelPath);
};
