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

using std::cout;
using std::endl;

// Test Fixtures
class ModelImportTest: public ::testing::Test {
 protected:
    void SetUp() override {
        modelImport = new ModelImport("dummy", texturePathStage, texturePatternStage, 0);
    }
    vector<string> texturePathStage;
    vector<GLint> texturePatternStage;
    ModelImport *modelImport;
};

TEST_F(ModelImportTest, GivenObjData_WhenProcessLinesForPolygon_ThenPolygonHasCorrectVertexData) {
    // Preparation
    Polygon *polygon = new Polygon();
    auto currentObject = 0;

    // Action
    for (auto it = fakeObjFile.begin(); it != fakeObjFile.end(); ++it) {
        currentObject = modelImport->processLine(*it, currentObject, polygon, false);
    }

    // Validation
    ASSERT_NE(nullptr, polygon);
    ASSERT_GT(polygon->vertices.size(), 0);

    // Checking vertices at start, middle and end
    ASSERT_FLOAT_EQ(-23.3, polygon->vertices[0][0]);
    ASSERT_FLOAT_EQ(24.039999, polygon->vertices[0][1]);
    ASSERT_FLOAT_EQ(-25.859999, polygon->vertices[0][2]);

    ASSERT_FLOAT_EQ(-19.260, polygon->vertices[0][63]);
    ASSERT_FLOAT_EQ(24.039999, polygon->vertices[0][64]);
    ASSERT_FLOAT_EQ(25.859999, polygon->vertices[0][65]);

    ASSERT_FLOAT_EQ(-19.260, polygon->vertices[0][105]);
    ASSERT_FLOAT_EQ(-22.039999, polygon->vertices[0][106]);
    ASSERT_FLOAT_EQ(-25.859999, polygon->vertices[0][107]);
}

TEST_F(ModelImportTest, GivenObjData_WhenProcessLinesForPolygon_ThenPolygonHasNormalizedTextureCoordinates) {
    
}

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


