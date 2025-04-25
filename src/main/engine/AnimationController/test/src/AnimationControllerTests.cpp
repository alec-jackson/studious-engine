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
#include <map>
#include <string>
#include <SpriteObject.hpp>
#include <MockGfxController.hpp>
#include <AnimationControllerTests.hpp>

using testing::_;

const unsigned int DUMMY_TEXTURE_ID = 0xDEADBEEF;
const unsigned int DUMMY_VAO = 0xBEEF;
const char *DUMMY_OBJ_NAME = "test object";
const char *DUMMY_TRACK_NAME = "test track";
const vector<int> REFERENCE_TRACK = { 3, 4, 5, 6, 5, 4 };
const int TARGET_FPS = 12;
const int INDEX_SHIFT = 1;

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
    /**
     * We'll probably want to do something else long term. I'm not mocking SpriteObjects because they are not easily
     * mocked in their current state.
    */
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
        *textureId = DUMMY_TEXTURE_ID;
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
        *vao = DUMMY_VAO;
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
    /* Real SpriteObject is used for now, but can be mocked later for less required test setup. */
    spriteObject_ = std::make_unique<SpriteObject>(
        testSpritePath, vec3(0, 0, 0), 1.0f, 0, DUMMY_OBJ_NAME, ObjectType::SPRITE_OBJECT,
        ObjectAnchor::TOP_LEFT, &mockGfxController_);

    /* Call split grid on the sprite object */
    spriteObject_->splitGrid(width, height, numFrames);
}

void GivenAnAnimationController::TearDown() {
    // Probably do something useful here
}

class GivenAnAnimationControllerReady : public GivenAnAnimationController, public ::testing::Test {
 public:
    void SetUp() override {
        GivenAnAnimationController::SetUp();
    }
    void TearDown() override {
        GivenAnAnimationController::TearDown();
    }
};

/**
 * @brief Ensures that the AnimationController removes active tracks when their complete sequence time
 * has elapsed. The completed animation track should leave the animation on the final frame of the track
 * when it completes.
 */
TEST_F(GivenAnAnimationControllerReady, WhenNoLoopAnimationFinished_ThenActiveTrackRemoved) {
    /* Preparation */
    // The expected frame should be the last in the track
    auto expectedFrame = REFERENCE_TRACK.at(REFERENCE_TRACK.size() - 1);
    auto expectedIdx = REFERENCE_TRACK.size();
    animationController_.addTrack(spriteObject_.get(), DUMMY_TRACK_NAME, REFERENCE_TRACK, TARGET_FPS, false);

    /* Action */
    animationController_.playTrack(DUMMY_TRACK_NAME);
    // The middle frame should appear between 33-66% of the animation's cycle
    deltaTime = (1.0 / TARGET_FPS) * expectedIdx;
    animationController_.update();

    /* Validation */
    // There should be no active tracks at this point
    auto activeTracks = animationController_.getActiveTracks();
    EXPECT_TRUE(activeTracks.empty());
    // The sprite should be left on the final frame when the animation completes
    EXPECT_EQ(spriteObject_->getCurrentFrame(), expectedFrame);
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
 * @brief Ensures that animation playback works as expected. Test parameter is the expected frame and the expected
 * index. The deltaTime is calculated using the expected index, which should prove that frame pacing is working as
 * expected as well.
 */
TEST_P(GivenAnAnimationControllerPlaybackParam, WhenTrackPlayingAndUpdateCalled_ThenExpectedFrameShown) {
    /* Preparation */
    auto expectedFrame = std::get<0>(GetParam());
    auto expectedIdx = std::get<1>(GetParam());
    animationController_.addTrack(spriteObject_.get(), DUMMY_TRACK_NAME, REFERENCE_TRACK, TARGET_FPS, true);

    /* Action */
    animationController_.playTrack(DUMMY_TRACK_NAME);
    // The middle frame should appear between 33-66% of the animation's cycle
    deltaTime = (1.0 / TARGET_FPS) * expectedIdx;
    animationController_.update();

    /* Validation */
    // Check the active tracks
    auto activeTracks = animationController_.getActiveTracks();
    EXPECT_EQ(activeTracks.size(), 1);
    EXPECT_EQ(activeTracks[DUMMY_OBJ_NAME]->state, TrackState::RUNNING);
    EXPECT_EQ(activeTracks[DUMMY_OBJ_NAME]->currentTrackIdx, expectedIdx % REFERENCE_TRACK.size());
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

class GivenAnAnimationControllerToTestRunning : public GivenAnAnimationControllerToTest {
 public:
    void SetUp() override {
        /* Preparation */
        GivenAnAnimationControllerToTest::SetUp();
        animationController_.addTrack(spriteObject_.get(), DUMMY_TRACK_NAME, REFERENCE_TRACK, TARGET_FPS, false);
        animationController_.playTrack(DUMMY_TRACK_NAME);
        // Make sure the animation is in a running state after adding
        validateActiveTracks(1, TrackState::RUNNING, 0);
    }

    void TearDown() override {
        GivenAnAnimationControllerToTest::TearDown();
    }

    void validateActiveTracks(uint size, TrackState state, int idx) {
        EXPECT_EQ(activeTracks_.size(), size);
        EXPECT_EQ(activeTracks_.at(DUMMY_OBJ_NAME).get()->state, state);
        EXPECT_EQ(activeTracks_.at(DUMMY_OBJ_NAME).get()->currentTrackIdx, idx);
    }
 protected:
    const map<string, std::shared_ptr<ActiveTrackEntry>> &activeTracks_ = animationController_.getActiveTracks();
};

/**
 * @brief Ensures that pausing an active animation will indeed pause it, and that the paused animation track
 * will no longer progress when update() is called.
 */
TEST_F(GivenAnAnimationControllerToTestRunning, WhenUpdatingWithPausedAnimation_ThenTrackNotUpdated) {
    /* Action */
    animationController_.pauseTrack(DUMMY_TRACK_NAME);
    deltaTime = (1.0 / TARGET_FPS) * INDEX_SHIFT;  // Shift track idx by 1
    animationController_.update();

    /* Validation */
    validateActiveTracks(1, TrackState::PAUSED, 0);
}

/**
 * @brief Ensures that paused animations that are resumed continue to be updated when update() is called.
 */
TEST_F(GivenAnAnimationControllerToTestRunning, WhenAnimationPausedThenResumed_ThenUpdatesResume) {
    /* Action */
    animationController_.pauseTrack(DUMMY_TRACK_NAME);
    deltaTime = (1.0 / TARGET_FPS) * INDEX_SHIFT;  // Shift track idx by 1
    animationController_.update();
    animationController_.playTrack(DUMMY_TRACK_NAME);
    animationController_.update();

    /* Validation */
    validateActiveTracks(1, TrackState::RUNNING, INDEX_SHIFT);
}

/**
 * @brief Ensures that resumed animations continue where they left off.
 */
TEST_F(GivenAnAnimationControllerToTestRunning, WhenPausedInMiddleOfPlayback_ThenResumedInPreviousPlace) {
    /* Preparation */
    int framesPassed = 2;
    deltaTime = (1.0 / TARGET_FPS) * INDEX_SHIFT * framesPassed;
    animationController_.update();
    /* Make sure the animation updated (sanity check) */
    EXPECT_EQ(spriteObject_.get()->getCurrentFrame(), REFERENCE_TRACK.at(framesPassed));
    EXPECT_EQ(activeTracks_.at(DUMMY_OBJ_NAME).get()->currentTrackIdx, framesPassed);

    /* Action */
    /* Pause/Play animation */
    animationController_.pauseTrack(DUMMY_TRACK_NAME);
    animationController_.playTrack(DUMMY_TRACK_NAME);

    /* Validation - Make sure track is where it left off */
    validateActiveTracks(1, TrackState::RUNNING, INDEX_SHIFT * framesPassed);
}

