/**
 * @file PolygonTests.cpp
 * @author Christian Galvez
 * @brief Unit test suite for the Polygon class
 * @version 0.1
 * @date 2023-12-02
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <gtest/gtest.h>
#include <iostream>
#include <PolygonTests.hpp>

using std::cout;
using std::endl;

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

Polygon createTestPolygon() {
    auto expectedPointCount = 123;
    auto expectedProgramId = 1;
    vector<GLfloat> expectedVertices = { 0.333, 0.694, 0.777 };
    vector<GLfloat> expectedTextures = { 1.0, 0.1, 0.0 };
    vector<GLfloat> expectedNormals = { 0.123, 0.234, 0.345 };

    auto polygon = Polygon(expectedPointCount, expectedProgramId, expectedVertices, expectedTextures, expectedNormals);

    polygon.shapeBufferId[0] = 7;
    polygon.normalBufferId[0] = 8;
    polygon.textureId[0] = 9;
    polygon.textureCoordsId[0] = 10;

    return polygon;
}

bool vectorEquals(vector<GLfloat> expectedVertices, vector<GLfloat> actualVertices) {
    auto result = true;
    if (expectedVertices.size() != actualVertices.size()) return false;
    for (int i = 0; i < expectedVertices.size(); i++) {
        if (expectedVertices[i] != actualVertices[i]) result = false;
    }
    return result;
}

TEST(PolygonConstructor, WhenConstructedWithData_ThenPolygonHasExpectedData) {
    // Preparation
    auto expectedPointCount = 123;
    auto expectedProgramId = 1;
    auto expectedShapeBufferId = 0;
    auto expectedNormalBufferId = 0;
    auto expectedNumberOfObjects = 1;
    vector<GLfloat> expectedVertices = { 0.333, 0.694, 0.777 };
    vector<GLfloat> expectedTextures = { 1.0, 0.1, 0.0 };
    vector<GLfloat> expectedNormals = { 0.123, 0.234, 0.345 };

    // Action
    auto polygon = Polygon(expectedPointCount, expectedProgramId, expectedVertices, expectedTextures, expectedNormals);

    // Validation
    EXPECT_EQ(expectedPointCount, polygon.pointCount[0]);
    EXPECT_EQ(expectedProgramId, polygon.programId);
    EXPECT_TRUE(vectorEquals(expectedVertices, polygon.vertices[0]));
    EXPECT_TRUE(vectorEquals(expectedTextures, polygon.textureCoords[0]));
    EXPECT_TRUE(vectorEquals(expectedNormals, polygon.normalCoords[0]));
    EXPECT_EQ(expectedShapeBufferId, polygon.shapeBufferId[0]);
    EXPECT_EQ(expectedNormalBufferId, polygon.normalBufferId[0]);
    EXPECT_EQ(expectedNumberOfObjects, polygon.numberOfObjects);
}

TEST(PolygonMerge, WhenTwoPolygonsMerged_ThenPolygonDataMergedOk)
{
    // Preparation
    auto expectedProgramId = 1;
    auto expectedNumberOfObjects = 2;
    auto expectedVectorSizesAfterMerge = 2;
    // Create two normal polygons to merge
    auto poly1 = createTestPolygon();
    auto poly2 = createTestPolygon();

    vector<GLfloat> expectedVertices = { 0.333, 0.694, 0.777 };
    vector<GLfloat> expectedTextures = { 1.0, 0.1, 0.0 };
    vector<GLfloat> expectedNormals = { 0.123, 0.234, 0.345 };

    // Action
    poly1.merge(poly2);

    // Validation
    // Check that the programId is the same after merge
    EXPECT_EQ(expectedProgramId, poly1.programId);

    // Check that the number of objects was incremented
    EXPECT_EQ(expectedNumberOfObjects, poly1.numberOfObjects);

    // Check that the vertex/texture/normal coordinates have been merged
    EXPECT_TRUE(vectorEquals(expectedVertices, poly1.vertices[0]));
    EXPECT_TRUE(vectorEquals(expectedVertices, poly1.vertices[1]));
    EXPECT_EQ(expectedVectorSizesAfterMerge, poly1.vertices.size());

    EXPECT_TRUE(vectorEquals(expectedTextures, poly1.textureCoords[0]));
    EXPECT_TRUE(vectorEquals(expectedTextures, poly1.textureCoords[1]));
    EXPECT_EQ(expectedVectorSizesAfterMerge, poly1.textureCoords.size());

    EXPECT_TRUE(vectorEquals(expectedNormals, poly1.normalCoords[0]));
    EXPECT_TRUE(vectorEquals(expectedNormals, poly1.normalCoords[1]));
    EXPECT_EQ(expectedVectorSizesAfterMerge, poly1.normalCoords.size());
}
