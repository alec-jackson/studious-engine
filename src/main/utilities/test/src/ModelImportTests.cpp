/**
 * @file ModelImportTests.cpp
 * @author Christian Galvez
 * @brief Unit tests for the ModelImport class
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <gtest/gtest.h>
#include <iostream>
#include <ModelImportTests.hpp>
#define PI 3.14159265
using std::cout;
using std::endl;

// Test Fixtures
class ModelImportTest: public ::testing::Test {
 protected:
    void SetUp() override {
        modelImport = new ModelImport("dummy", texturePathStage, texturePatternStage, 0);
    }
    void TearDown() override {
    }
    vector<string> texturePathStage;
    vector<int> texturePatternStage;
    ModelImport *modelImport;
};

/**
 * @brief When parsing proper OBJ data, vertex data is properly built in polygon
 */
TEST_F(ModelImportTest, GivenObjData_WhenProcessLinesForPolygon_ThenPolygonHasCorrectVertexData) {
    // Preparation
    auto currentObject = 0;

    // Action
    for (auto it = fakeObjFile.begin(); it != fakeObjFile.end(); ++it) {
        currentObject = modelImport->processLine(*it, currentObject);
    }

    // Add an extra object line to create poly
    currentObject = modelImport->processLine("o something", currentObject);
    auto polygon = modelImport->getPolygon();

    // Validation
    ASSERT_EQ(108, polygon.vertices[0].size());

    // Spot checking vertices at start, middle and end
    ASSERT_FLOAT_EQ(-23.3, polygon.vertices[0][0]);
    ASSERT_FLOAT_EQ(24.039999, polygon.vertices[0][1]);
    ASSERT_FLOAT_EQ(-25.859999, polygon.vertices[0][2]);

    ASSERT_FLOAT_EQ(-19.260, polygon.vertices[0][63]);
    ASSERT_FLOAT_EQ(24.039999, polygon.vertices[0][64]);
    ASSERT_FLOAT_EQ(25.859999, polygon.vertices[0][65]);

    ASSERT_FLOAT_EQ(-19.260, polygon.vertices[0][105]);
    ASSERT_FLOAT_EQ(-22.039999, polygon.vertices[0][106]);
    ASSERT_FLOAT_EQ(-25.859999, polygon.vertices[0][107]);
}

/**
 * @brief When parsing proper OBJ data, texture coordinate data is properly built in polygon
 * 
 */
TEST_F(ModelImportTest, GivenObjData_WhenProcessLinesForPolygon_ThenPolygonHasNormalizedTextureCoordinates) {
    // Preparation
    auto currentObject = 0;

    // Action
    for (auto it = fakeObjFile.begin(); it != fakeObjFile.end(); ++it) {
        currentObject = modelImport->processLine(*it, currentObject);
    }

    // Add an extra object line to create poly
    currentObject = modelImport->processLine("o something", currentObject);
    auto polygon = modelImport->getPolygon();

    // Validation
    ASSERT_EQ(72, polygon.textureCoords[0].size());

    // Ensure that texture coords are all non-negative and between 0.0 and 1.0
    for (auto it = polygon.textureCoords[0].begin(); it != polygon.textureCoords[0].end(); ++it) {
        ASSERT_GE(*it, 0.0f);
        ASSERT_LE(*it, 1.0f);
    }

    // Spot checking texture coordinates at start, middle and end
    ASSERT_FLOAT_EQ(0.875000, polygon.textureCoords[0][0]);
    ASSERT_FLOAT_EQ(0.5, polygon.textureCoords[0][1]);
    ASSERT_FLOAT_EQ(0.625, polygon.textureCoords[0][2]);

    ASSERT_FLOAT_EQ(0.625, polygon.textureCoords[0][30]);
    // This value is 0.75 because we "normalize" the second coordinate (aka 1.0f - value)
    ASSERT_FLOAT_EQ(0.75, polygon.textureCoords[0][31]);
    ASSERT_FLOAT_EQ(0.375, polygon.textureCoords[0][32]);

    ASSERT_FLOAT_EQ(0.5, polygon.textureCoords[0][69]);
    ASSERT_FLOAT_EQ(0.375, polygon.textureCoords[0][70]);
    ASSERT_FLOAT_EQ(0.5, polygon.textureCoords[0][71]);
}

/**
 * @brief When given proper OBJ data, normal coordinates are properly built in polygon
 * 
 */
TEST_F(ModelImportTest, GivenObjData_WhenProcessLinesForPolygon_ThenPolygonHasCorrectNormals) {
    // Preparation
    auto currentObject = 0;

    // Action
    for (auto it = fakeObjFile.begin(); it != fakeObjFile.end(); ++it) {
        currentObject = modelImport->processLine(*it, currentObject);
    }

    // Add an extra object line to create poly
    currentObject = modelImport->processLine("o something", currentObject);
    auto polygon = modelImport->getPolygon();

    // Validation
    ASSERT_EQ(108, polygon.normalCoords[0].size());

    // Spot checking normals at start, middle and end
    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][0]);
    ASSERT_FLOAT_EQ(1.0, polygon.normalCoords[0][1]);
    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][2]);

    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][63]);
    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][64]);
    ASSERT_FLOAT_EQ(1.0, polygon.normalCoords[0][65]);

    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][105]);
    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][106]);
    ASSERT_FLOAT_EQ(-1.0, polygon.normalCoords[0][107]);
}

/**
 * @brief When parsing proper OBJ data with multiple objects, data in polygon (vertex, texture, normal) is properly set
 * 
 */
TEST_F(ModelImportTest, GivenTwoObjs_WhenProcessLinesForPolygon_ThenPolygonHasCorrectVertexData) {
    // Preparation
    auto currentObject = 0;

    // Action
    // Process same fake file twice to create two objects
    for (auto it = fakeObjFile.begin(); it != fakeObjFile.end(); ++it) {
        currentObject = modelImport->processLine(*it, currentObject);
    }

    for (auto it = fakeObjFile.begin(); it != fakeObjFile.end(); ++it) {
        currentObject = modelImport->processLine(*it, currentObject);
    }

    // Add an extra object line to create poly
    currentObject = modelImport->processLine("o something", currentObject);
    auto polygon = modelImport->getPolygon();

    // Validation
    ASSERT_EQ(2, polygon.vertices.size());  // Two sets of vertices
    ASSERT_EQ(2, polygon.textureCoords.size());  // Two sets of texture coords

    ASSERT_EQ(108, polygon.vertices[0].size());
    ASSERT_EQ(108, polygon.vertices[1].size());
    ASSERT_EQ(72, polygon.textureCoords[0].size());
    ASSERT_EQ(72, polygon.textureCoords[1].size());

    // Spot checking vertices at start, middle and end
    ASSERT_FLOAT_EQ(-23.3, polygon.vertices[0][0]);
    ASSERT_FLOAT_EQ(-23.3, polygon.vertices[1][0]);

    ASSERT_FLOAT_EQ(24.039999, polygon.vertices[0][64]);
    ASSERT_FLOAT_EQ(24.039999, polygon.vertices[1][64]);

    ASSERT_FLOAT_EQ(-25.859999, polygon.vertices[0][107]);
    ASSERT_FLOAT_EQ(-25.859999, polygon.vertices[1][107]);

    // Spot checking texture coordinates at start, middle and end
    ASSERT_FLOAT_EQ(0.875000, polygon.textureCoords[0][0]);
    ASSERT_FLOAT_EQ(0.875000, polygon.textureCoords[1][0]);

    // This value is 0.75 because we "normalize" the second coordinate (aka 1.0f - value)
    ASSERT_FLOAT_EQ(0.75, polygon.textureCoords[0][31]);
    ASSERT_FLOAT_EQ(0.75, polygon.textureCoords[1][31]);

    ASSERT_FLOAT_EQ(0.5, polygon.textureCoords[0][71]);
    ASSERT_FLOAT_EQ(0.5, polygon.textureCoords[1][71]);

    // Spot checking normal coordinates at start, middle and end
    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][0]);
    ASSERT_FLOAT_EQ(0.0, polygon.normalCoords[0][0]);

    ASSERT_FLOAT_EQ(1.0, polygon.normalCoords[0][65]);
    ASSERT_FLOAT_EQ(1.0, polygon.normalCoords[0][65]);

    ASSERT_FLOAT_EQ(-1.0, polygon.normalCoords[0][107]);
    ASSERT_FLOAT_EQ(-1.0, polygon.normalCoords[0][107]);
}

/**
 * @brief Launches google test suite defined in file
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    cout << "Running GTESTS" << endl;
    auto result = RUN_ALL_TESTS();
    if (!result) {
        cout << "All tests passed" << endl;
    } else {
        cout << "Some test failures detected!" << endl;
    }

    return result;
}
