/**
 * @file AnimationControllerTests.cpp
 * @brief Unit test suite for the AnimationController class.
 * @copyright Copyright (c) 2025
 * @author Christian Galvez
 * @date 2025-04-13
 */
#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <iostream>
#include <tuple>
#include <SpriteObject.hpp>
#include <MockGfxController.hpp>
#include <AnimationControllerTests.hpp>

using testing::_;

const unsigned int dummyTextureId = 0xDEADBEEF;
const unsigned int dummyVao = 0xBEEF;
double deltaTime;
const vector<int> referenceTrack = { 3, 4, 5, 6, 5, 4 };

const char *testSpritePath = "../src/resources/images/test_image.png";
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

class GivenAnAnimationController {
 public:
    void SetUp();
    void TearDown();
    void initMocks();
 protected:
    AnimationController animationController_;
    std::unique_ptr<SpriteObject> spriteObject_;
    testing::NiceMock<MockGfxController> mockGfxController_;
    int width = 5;
    int height = 4;
    int bytesPerPixel = 3;
    int imageWidth = 30;
    int imageHeight = 16;
    int numFrames = 24;
    std::vector<std::shared_ptr<uint8_t[]>> actualFrames;
};

/**
 * @brief Initializes mocks for the GfxController. Most mocks just define default behavior, but
 * the EXPECT_CALL mocks will capture generated frame data from the SpriteObject.
 * 
 */
void GivenAnAnimationController::initMocks() {
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

    /* These mocks will capture generated frame data */
    EXPECT_CALL(mockGfxController_, sendTextureData(_, _, _, _))
        .Times(numFrames + 1)
        .WillOnce([this](unsigned int w, unsigned int h,
            TexFormat format, void *data) {
            EXPECT_EQ(w, imageWidth);
            EXPECT_EQ(h, imageHeight);
            EXPECT_EQ(format, TexFormat::RGB);
            return GFX_OK(unsigned int);
        })
        .WillRepeatedly([this](unsigned int w, unsigned int h,
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
}

void GivenAnAnimationController::SetUp() {
    initMocks();
    spriteObject_ = std::make_unique<SpriteObject>(
        testSpritePath, vec3(0, 0, 0), 1.0f, 0, "test object", ObjectType::GAME_OBJECT_2D,
        ObjectAnchor::TOP_LEFT, &mockGfxController_);

    /* Call split grid on the sprite object */
    spriteObject_->splitGrid(width, height, numFrames);
}

void GivenAnAnimationController::TearDown() {
    // Probably do something useful here
}

class GivenAnAnimationControllerPlaybackParam : public GivenAnAnimationController,
    public ::testing::TestWithParam<std::tuple<int, int>> {
 public:
    void SetUp() override {
        GivenAnAnimationController::SetUp();
    }

    void TearDown() override {
        GivenAnAnimationController::TearDown();
    }
};

/**
 * @brief Ensures that animation playback works as expected.
 */
TEST_P(GivenAnAnimationControllerPlaybackParam, WhenTrackPlayingAndUpdateCalled_ThenExpectedFrameShown) {
    /* Preparation */
    auto targetFps = 12;
    auto expectedFrame = std::get<0>(GetParam());
    auto expectedIdx = std::get<1>(GetParam());
    animationController_.addTrack(spriteObject_.get(), "test track", referenceTrack, targetFps);

    /* Action */
    animationController_.playTrack("test track");
    // The middle frame should appear between 33-66% of the animation's cycle
    deltaTime = (1.0 / targetFps) * expectedIdx;
    animationController_.update();

    /* Validation */
    // Check the active tracks
    auto activeTracks = animationController_.getActiveTracks();
    EXPECT_EQ(activeTracks.size(), 1);
    EXPECT_EQ(activeTracks["test object"]->state, TrackState::RUNNING);
    EXPECT_EQ(activeTracks["test object"]->currentTrackIdx, expectedIdx % referenceTrack.size());
    EXPECT_EQ(spriteObject_->getCurrentFrame(), expectedFrame);
}

INSTANTIATE_TEST_SUITE_P(PlayTrackTests, GivenAnAnimationControllerPlaybackParam,
    ::testing::Values(
        // Tuple order: 1 - expected frame, 2 - expected index
        std::make_tuple(3, 0),
        std::make_tuple(4, 1),
        std::make_tuple(5, 2),
        std::make_tuple(6, 3),
        std::make_tuple(5, 4),
        std::make_tuple(4, 5),
        std::make_tuple(3, 6)  // Should wrap around to the first frame
    )
);

class GivenAnAnimationControllerToTest : public GivenAnAnimationController, public ::testing::Test {
 public:
    void SetUp() override {
        GivenAnAnimationController::SetUp();
    }

    void TearDown() override {
        GivenAnAnimationController::TearDown();
    }
};

TEST_F(GivenAnAnimationControllerToTest, WhenPausingARunningAnimation_ThenTrackIsPaused) {
    /* Preparation */
    auto targetFps = 12;
    animationController_.addTrack(spriteObject_.get(), "test track", referenceTrack, targetFps);
    animationController_.playTrack("test track");

    auto activeTracks = animationController_.getActiveTracks();
    // Make sure the animation is in a running state after adding
    EXPECT_EQ(activeTracks.size(), 1);
    EXPECT_EQ(activeTracks["test object"].get()->state, TrackState::RUNNING);

    /* Action */
    animationController_.pauseTrack("test object");

    /* Validation */
    EXPECT_EQ(activeTracks.size(), 1);
    EXPECT_EQ(activeTracks["test object"].get()->state, TrackState::PAUSED);
}

TEST_F(GivenAnAnimationControllerToTest, WhenUpdatingWithPausedAnimation_ThenTrackNotUpdated) {
    /* Preparation */
    auto targetFps = 12;
    auto indexShift = 1;
    animationController_.addTrack(spriteObject_.get(), "test track", referenceTrack, targetFps);
    animationController_.playTrack("test track");
    auto activeTracks = animationController_.getActiveTracks();
    EXPECT_EQ(activeTracks["test object"].get()->currentTrackIdx, 0);

    /* Action */
    animationController_.pauseTrack("test object");
    deltaTime = (1.0 / targetFps) * indexShift;  // Shift track idx by 1
    animationController_.update();

    /* Validation */
    EXPECT_EQ(activeTracks.size(), 1);
    EXPECT_EQ(activeTracks["test object"].get()->state, TrackState::PAUSED);
    EXPECT_EQ(activeTracks["test object"].get()->currentTrackIdx, 0);
}

TEST_F(GivenAnAnimationControllerToTest, WhenAnimationPausedThenResumed_ThenUpdatesResume) {
    /* Preparation */
    auto targetFps = 12;
    auto indexShift = 1;
    animationController_.addTrack(spriteObject_.get(), "test track", referenceTrack, targetFps);
    animationController_.playTrack("test track");
    auto activeTracks = animationController_.getActiveTracks();
    EXPECT_EQ(activeTracks["test object"].get()->currentTrackIdx, 0);

    /* Action */
    animationController_.pauseTrack("test object");
    deltaTime = (1.0 / targetFps) * indexShift;  // Shift track idx by 1
    animationController_.update();
    animationController_.playTrack("test track");
    animationController_.update();

    /* Validation */
    EXPECT_EQ(activeTracks.size(), 1);
    EXPECT_EQ(activeTracks["test object"].get()->state, TrackState::RUNNING);
    EXPECT_EQ(activeTracks["test object"].get()->currentTrackIdx, indexShift);
}

