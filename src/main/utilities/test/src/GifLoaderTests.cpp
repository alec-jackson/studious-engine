/**
 * @file GifLoaderTests.cpp
 * @brief Test suite for GifLoader class functionality.
 * @author Christian Galvez
 * @copyright Copyright (c) 2025
 */
#include <GifLoaderTests.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <cstdio>

using std::cout;
using std::endl;

const char *imagePath = "../src/resources/images/dne.gif";
const char *testImage = "../src/resources/images/sample_1.gif";
const char *animatedImage = "../src/resources/images/sample_2_animation.gif";
const char *bigAnimatedImage = "../src/resources/images/dne.gif";
const char *smallAnimatedImage = "../src/resources/images/150100.gif";

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
    ASSERT_EQ(10, gifLoader.getCanvasWidth());
    ASSERT_EQ(10, gifLoader.getCanvasHeight());

    ASSERT_EQ(1, gifLoader.getGlobalColorTableFlag());
    ASSERT_EQ(1, gifLoader.getColorResolution());
    ASSERT_EQ(0, gifLoader.getSortFlag());
    ASSERT_EQ(1, gifLoader.getGlobalColorTableSize());
    ASSERT_EQ(0, gifLoader.getBackgroundColorIndex());
    ASSERT_EQ(1, gifLoader.getPixelAspectRatio());

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

TEST(GifLoaderTest, WhenImageLoaded_ThenRawImageDataAsExpected) {
    // Preparation / Action
    auto gifLoader = GifLoader(testImage);

    // Validate the raw image data
    auto image = gifLoader.getImage(0);
    // auto imageData = image.imageData;

    // Validate the first image
    ASSERT_EQ(10, image.imageWidth);
    ASSERT_EQ(10, image.imageHeight);

    // Validate the raw image data
    auto imageSize = image.imageWidth * image.imageHeight;
    ASSERT_EQ(100, imageSize);

    auto checkCount = 0;
    for (int i = 0; i < imageSize * 3; i++) {
        ASSERT_EQ(referenceImageData[i], image.imageData[i]);
        checkCount++;
    }

    ASSERT_EQ(300, checkCount);
}

TEST(GifLoaderTest, WhenAnimatedImageLoaded_ThenMultipleImagesPresent) {
    // Preparation / Action
    auto gifLoader = GifLoader(animatedImage);

    // Validate the raw image data
    auto images = gifLoader.getImages();
    ASSERT_EQ(3, images.size());

    // Validate that the first image has the expected height and width
    auto firstImage = images.at(0);
    ASSERT_EQ(11, firstImage.imageWidth);
    ASSERT_EQ(29, firstImage.imageHeight);

    // Validate that the second and third images have a cropped width and height
    auto secondImage = images.at(1);
    auto thirdImage = images.at(2);
    // The image width/heights should be updated to match the reference image
    ASSERT_EQ(11, secondImage.imageWidth);
    ASSERT_EQ(29, secondImage.imageHeight);

    ASSERT_EQ(11, thirdImage.imageWidth);
    ASSERT_EQ(29, thirdImage.imageHeight);

    // Make sure the additional images do not start at origin
    ASSERT_EQ(2, secondImage.imageLeft);
    ASSERT_EQ(11, secondImage.imageTop);

    ASSERT_EQ(2, thirdImage.imageLeft);
    ASSERT_EQ(2, thirdImage.imageTop);

    // Validate the first image starts at origin
    ASSERT_EQ(0, firstImage.imageLeft);
    ASSERT_EQ(0, firstImage.imageTop);
}

TEST(GifLoaderTest, WhenBigAnimatedImageLoaded_ThenImageDataCorrect) {
    // Preparation / Action
    auto gifLoader = GifLoader(smallAnimatedImage);
    printf("Testdone\n");
    // ASSERT_EQ(0, 1);
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
