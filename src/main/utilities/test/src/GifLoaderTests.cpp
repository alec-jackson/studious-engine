#include <GifLoaderTests.hpp>
#include <gtest/gtest.h>
#include <iostream>

using std::cout;
using std::endl;

string imagePath = "../src/resources/images/Shrek.gif";
string testImage = "../src/resources/images/giflib-logo.gif";

/*
// Test Fixtures
class GifLoaderTest: public ::testing::Test {
 protected:
    void SetUp() override {
        gifLoader = new GifLoader(imagePath);
    }
    void TearDown() override {
        delete gifLoader;
    }

    GifLoader *gifLoader;
};*/

/**
 * @brief When parsing proper OBJ data, vertex data is properly built in polygon
 */
/*
TEST(GifLoaderTest, WhenOpenShrek_ThenTestValuesAsExpected) {
    // Preparation / Action
    auto gifLoader = GifLoader(imagePath);

    // Validation
    ASSERT_EQ(GifVersion::GIF89a, gifLoader.getVersion());
    ASSERT_EQ(500, gifLoader.getCanvasWidth());
    ASSERT_EQ(280, gifLoader.getCanvasHeight());

    ASSERT_EQ(1, gifLoader.getGlobalColorTableFlag());
    ASSERT_EQ(7, gifLoader.getColorResolution());
    ASSERT_EQ(0, gifLoader.getSortFlag());
    ASSERT_EQ(7, gifLoader.getGlobalColorTableSize());
    ASSERT_EQ(229, gifLoader.getBackgroundColorIndex());
    ASSERT_EQ(0, gifLoader.getPixelAspectRatio());

    // Validate GCE variables
    ASSERT_EQ(0x04, gifLoader.getGceBlockSize());
    ASSERT_EQ(0x04, gifLoader.getGceDelayTime());
    ASSERT_EQ(229, gifLoader.getGceTransparentColorIndex());

    // Validate the first image
    auto image = gifLoader.getImages().front();

    ASSERT_FALSE(image.localColorTableFlag);
    ASSERT_TRUE(image.interlaceFlag);
    ASSERT_FALSE(image.sortFlag);
    ASSERT_EQ(0x00, image.lctSize);
}*/

/**
 * @brief When parsing proper OBJ data, vertex data is properly built in polygon
 */
TEST(GifLoaderTest, WhenOpenTestImage_ThenPropertiesCorrect) {
    // Preparation / Action
    auto gifLoader = GifLoader(testImage);

    // Validation
    ASSERT_EQ(GifVersion::GIF89a, gifLoader.getVersion());
    ASSERT_EQ(50, gifLoader.getCanvasWidth());
    ASSERT_EQ(50, gifLoader.getCanvasHeight());

    ASSERT_EQ(1, gifLoader.getGlobalColorTableFlag());
    ASSERT_EQ(7, gifLoader.getColorResolution());
    ASSERT_EQ(0, gifLoader.getSortFlag());
    ASSERT_EQ(6, gifLoader.getGlobalColorTableSize());
    ASSERT_EQ(0, gifLoader.getBackgroundColorIndex());
    ASSERT_EQ(0, gifLoader.getPixelAspectRatio());

    // Validate GCE variables
    ASSERT_EQ(0x04, gifLoader.getGceBlockSize());
    ASSERT_EQ(0x00, gifLoader.getGceDelayTime());
    ASSERT_EQ(0, gifLoader.getGceTransparentColorIndex());

    // Validate the first image
    auto image = gifLoader.getImages().front();

    ASSERT_FALSE(image.localColorTableFlag);
    ASSERT_FALSE(image.interlaceFlag);
    ASSERT_FALSE(image.sortFlag);
    ASSERT_EQ(0x00, image.lctSize);
}

TEST(GifLoaderTest, WhenLzwCompressionRun_ThenTablesCorrect) {

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
