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
#include <string>
#include <JsonParserTests.hpp>
using std::cout;
using std::endl;

// Test Fixtures
class JsonParserTest: public ::testing::Test {
 protected:
    void SetUp() override {
    }
    void TearDown() override {
    }
};

TEST(JsonParserTest, WhenBasicObjectParsed_ThenReturnedJsonNodeIsValid) {
    // Preparation
    string jsonData = "{\"Hello\": \"World\"}";

    // Action
    auto res = parseJson(jsonData);

    // Validation
    ASSERT_TRUE(res != nullptr);
    ASSERT_TRUE(res->objectData["Hello"] != nullptr);
    ASSERT_EQ(res->objectData["Hello"]->data, "World");
}

TEST(JsonParserTest, WhenNestedObjectParsed_ThenReturnedJsonNodeHasNest) {
    // Preparation
    string jsonData = "{\"Hello\": { \"World\": \"Stuff\"}}";

    // Action
    auto res = parseJson(jsonData);

    // Validation
    ASSERT_TRUE(res != nullptr);
    ASSERT_TRUE(res->objectData["Hello"] != nullptr);
    ASSERT_TRUE(res->objectData["Hello"]->objectData["World"] != nullptr);
    ASSERT_EQ(res->objectData["Hello"]->objectData["World"]->data, "Stuff");
}
