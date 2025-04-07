/**
 * @file SpriteObjectTests.cpp
 * @author Christian Galvez
 * @brief Test suite for SpriteObject functions.
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <iostream>
#include <MockGfxController.hpp>
#include <SpriteObjectTests.hpp>

using ::testing::_;
using ::testing::Return;

const unsigned int dummyTextureId = 0xDEADBEEF;
const unsigned int dummyVao = 0xBEEF;


const char *testSpritePath = "src/resources/images/test_image.png";
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

class GivenASpriteObject: public ::testing::Test {
 protected:
    void initMocks();
    testing::NiceMock<MockGfxController> mockGfxController_;
};

void GivenASpriteObject::initMocks() {
    // Set up the mock GfxController
    ON_CALL(mockGfxController_, generateTexture(_)).WillByDefault([](unsigned int *textureId) {
        *textureId = dummyTextureId;
        return GFX_OK(unsigned int);
    });
    ON_CALL(mockGfxController_, bindTexture(_))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, sendTextureData(_, _, _, _))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, setTexParam(_, _))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, generateMipMap())
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, initVao(_)).WillByDefault([](unsigned int *vao) {
        *vao = dummyVao;
        return GFX_OK(unsigned int);
    });
    ON_CALL(mockGfxController_, bindVao(_))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, generateBuffer(_))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, bindBuffer(_))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, sendBufferData(_, _))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, enableVertexAttArray(_, _))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, setProgram(_))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, getShaderVariable(_, _))
        .WillByDefault(testing::Return(GFX_OK(int)));
}

/**
 * @brief Ensures that the split grid function correctly parses frames from a sprite grid.
 */
TEST_F(GivenASpriteObject, WhenSplitGridCalled_ThenImagesSplitSuccessfully) {
    /* Preparation */
    int width = 5;
    int height = 4;
    int bytesPerPixel = 3;
    int imageWidth = 30;
    int imageHeight = 16;
    int numFrames = 24;
    std::vector<std::shared_ptr<uint8_t[]>> actualFrames;
    initMocks();
    EXPECT_CALL(mockGfxController_, sendTextureData(_, _, _, _))
        .Times(numFrames + 1)
        .WillOnce([imageWidth, imageHeight](unsigned int w, unsigned int h,
            TexFormat format, void *data) {
            EXPECT_EQ(w, imageWidth);
            EXPECT_EQ(h, imageHeight);
            EXPECT_EQ(format, TexFormat::RGB);
            return GFX_OK(unsigned int);
        })
        .WillRepeatedly([&actualFrames, bytesPerPixel](unsigned int w, unsigned int h,
            TexFormat format, void *data) {
            EXPECT_EQ(w, 5);
            EXPECT_EQ(h, 4);
            EXPECT_EQ(format, TexFormat::RGB);
            std::shared_ptr<uint8_t[]> frameData(new uint8_t[w * h * bytesPerPixel], std::default_delete<uint8_t[]>());
            // Copy the memory into the frameData buffer
            memcpy(frameData.get(), data, w * h * bytesPerPixel);
            actualFrames.push_back(frameData);
            return GFX_OK(unsigned int);
        });

    SpriteObject sprite(testSpritePath, vec3(0.0f, 0.0f, 0.0f), 1.0f, 1, "testSprite", ObjectType::GAME_OBJECT_2D,
        ObjectAnchor::CENTER, &mockGfxController_);

    /* Action */
    sprite.splitGrid(width, height, numFrames);

    /* Validation */
    auto frameSize = width * height * bytesPerPixel;
    auto frameIndex = 0;
    for (auto frame : actualFrames) {
        // Ensure the memory between the actual and expected frames match
        auto result = memcmp(frame.get(), expectedFrameData.at(frameIndex++).data(), frameSize);
        EXPECT_EQ(0, result);
    }
}
