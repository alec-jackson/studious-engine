/**
 * @file AnimationControllerTests.cpp
 * @brief Unit test suite for the AnimationController class.
 * @copyright Copyright (c) 2025
 * @author Christian Galvez
 * @date 2025-04-13
 */
#include <AnimationControllerTests.hpp>
#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <iostream>
#include <tuple>
#include <map>
#include <string>
#include <SpriteObject.hpp>
#include <MockGfxController.hpp>
#include <DummyGfxController.hpp>

using testing::_;

const unsigned int DUMMY_TEXTURE_ID = 0xDEADBEEF;
const unsigned int DUMMY_VAO = 0xBEEF;
const char *DUMMY_OBJ_NAME = "test object";
const char *DUMMY_TRACK_NAME = "test track";
const vector<int> REFERENCE_TRACK = { 3, 4, 5, 6, 5, 4 };
const int TARGET_FPS = 12;
const int INDEX_SHIFT = 1;

const char *testSpritePath = "../src/resources/images/test_image.png";
const char *testFontPath = "../src/resources/fonts/AovelSans.ttf";

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

template<typename T>
void ASSERT_VEC_EQ(T actual, T expected) {
    auto containerSize = sizeof(T) / sizeof(float);
    for (uint i = 0; i < containerSize; ++i) {
        ASSERT_FLOAT_EQ(actual[i], expected[i]);
    }
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
    DummyGfxController dummyGfxController_;  // When mocks aren't important
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
    ON_CALL(mockGfxController_, bindTexture(_, _))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, sendTextureData(_, _, _, _))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, setTexParam(_, _, _))
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
    ON_CALL(mockGfxController_, enableVertexAttArray(_, _, _, _))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, setProgram(_))
        .WillByDefault(testing::Return(GFX_OK(unsigned int)));
    ON_CALL(mockGfxController_, getShaderVariable(_, _))
        .WillByDefault(testing::Return(GFX_OK(int)));

    /* These mocks will capture generated frame data */
    EXPECT_CALL(mockGfxController_, sendTextureData(_, _, _, _))
        .Times(numFrames + 1)
        .WillOnce([this](unsigned int w, unsigned int h,
            TexFormat format, [[maybe_unused]]void *data) {
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
    spriteObject_->createAnimation(width, height, numFrames);
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

/**
 * @brief Ensures that float values are updated as expected for linear transformations. The scenario for updating the
 * float value is described below:
 *
 * The current "keyframe" is created with the following parameters:
 * - Original value = 1.0f
 * - Target value = 5.0f
 * - Target time = 2.0f
 * - Current time = 0.5f
 *
 * What all of this means...
 * - The keyframe was created with a float value of 1.0f. This represents the original state of the target SceneObject
 *   for some attribute when the keyframe was created. This is tracked in the keyframe as a reference point for future
 *   transformations.
 * - The keyframe will transform the float value into whatever was set as the target value. Here, the target value of
 *   5.0f means the transformation (keyframe) WILL end with the float being 5.0f if the appropriate amount of time has
 *   passed.
 * - Target time is the amount of time the keyframe should process for. In this instance, a 2.0f second keyframe will
 *   complete after two seconds have passed.
 * - Current time is the current running some of the time that has passed in the keyframe so far. In this example, the
 *   keyframe has already processed for a grand total of 0.5f seconds. This should never be greater than the target
 *   time value after calls to update() complete.
 */
TEST_F(GivenAnAnimationControllerReady, WhenUpdateFloatCalled_ThenFloatUpdated) {
    /* Preparation */
    float originalValue = 1.0f;
    float desiredValue = 5.0f;
    float kfTargetTime = 2.0f;
    float kfCurrentTime = 0.5f;
    float expectedTransformation = originalValue + ((desiredValue - originalValue) * (kfCurrentTime / kfTargetTime));
    auto keyframe = AnimationController::createKeyFrame(UPDATE_NONE, kfTargetTime);
    keyframe->currentTime = kfCurrentTime;

    /* Action */
    auto result = animationController_.updateFloat(originalValue, desiredValue, keyframe.get());

    /* Validation */
    // Ensure the update work as we expected it to
    ASSERT_FALSE(result.updateComplete_);
    ASSERT_EQ(expectedTransformation, result.updatedValue_);
}

/**
 * @brief Ensures that keyframes are marked as completed when the exact amount of time set in the target time
 * has past.
 */
TEST_F(GivenAnAnimationControllerReady, WhenUpdateFloatFinishesKeyFrameExactly_ThenUpdateDataIsComplete) {
    /* Preparation */
    float originalValue = 1.0f;
    float desiredValue = 5.0f;
    float kfTargetTime = 2.0f;
    auto keyframe = AnimationController::createKeyFrame(UPDATE_NONE, kfTargetTime);
    keyframe->currentTime = kfTargetTime;

    /* Action */
    auto result = animationController_.updateFloat(originalValue, desiredValue, keyframe.get());

    /* Validation */
    // Ensure the update work as we expected it to
    ASSERT_TRUE(result.updateComplete_);
    ASSERT_EQ(desiredValue, result.updatedValue_);
    // Check that the keyframe's current time was updated
    ASSERT_EQ(kfTargetTime, keyframe->currentTime);
}

/**
 * @brief Ensures that a keyframe added to the animation controller will be present in the keyframe store for the
 * target object's name.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenFirstKeyFrameAddedForObject_ThenKeyFramePresentInStore) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float expTime = 1.0f;
    int expKfStoreSize = 1;
    int expQueueSize = 1;
    auto keyFrame = AnimationController::createKeyFrame(UPDATE_NONE, expTime);

    /* Action */
    animationController_.addKeyFrame(&obj, keyFrame);

    /* Validation */
    auto &kfStore = animationController_.getKeyFrameStore();
    ASSERT_EQ(expKfStoreSize, kfStore.size());
    ASSERT_TRUE(kfStore.find(DUMMY_OBJ_NAME) != kfStore.end());
    // The keyframe should exist in the queue for the dummy object
    ASSERT_EQ(expQueueSize, kfStore.at(DUMMY_OBJ_NAME).kQueue.size());
    // Check that the keyframe data looks correct
    ASSERT_EQ(expTime, kfStore.at(DUMMY_OBJ_NAME).kQueue.front().get()->targetTime);
    ASSERT_EQ(UPDATE_NONE, kfStore.at(DUMMY_OBJ_NAME).kQueue.front().get()->type);
}

/**
 * @brief Ensures that we can add more than one object to the keyframe queue.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenSecondKeyFrameAddedForObject_ThenBothKeyFramesPresentInStore) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float expTimeFirst = 1.0f;
    float expTimeSecond = 2.0f;
    int expKfStoreSize = 1;
    int expQueueSize = 2;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_NONE, expTimeFirst);
    auto keyFrame_2 = AnimationController::createKeyFrame(UPDATE_POS, expTimeSecond);

    /* Action */
    animationController_.addKeyFrame(&obj, keyFrame_1);
    animationController_.addKeyFrame(&obj, keyFrame_2);

    /* Validation */
    auto &kfStore = animationController_.getKeyFrameStore();
    ASSERT_EQ(expKfStoreSize, kfStore.size());
    ASSERT_TRUE(kfStore.find(DUMMY_OBJ_NAME) != kfStore.end());
    // The keyframe should exist in the queue for the dummy object
    ASSERT_EQ(expQueueSize, kfStore.at(DUMMY_OBJ_NAME).kQueue.size());
    // Check that the keyframe data looks correct
    ASSERT_EQ(expTimeFirst, kfStore.at(DUMMY_OBJ_NAME).kQueue.front().get()->targetTime);
    ASSERT_EQ(UPDATE_NONE, kfStore.at(DUMMY_OBJ_NAME).kQueue.front().get()->type);
    ASSERT_EQ(expTimeSecond, kfStore.at(DUMMY_OBJ_NAME).kQueue.back().get()->targetTime);
    ASSERT_EQ(UPDATE_POS, kfStore.at(DUMMY_OBJ_NAME).kQueue.back().get()->type);
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (FLOAT UPDATE).
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameUpdates_ThenFloatTransformationOccursForObject) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float originalScale = 1.0f;
    float desiredScale = 9.0f;
    float targetTime = 3.0f;
    deltaTime = 1.0f;
    float expectedTransformation = originalScale + ((desiredScale - originalScale) * (deltaTime / targetTime));
    int expectedKeyFrames = 1;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);

    obj.setScale(originalScale);
    keyFrame_1.get()->scale.desired = desiredScale;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_EQ(expectedTransformation, obj.getScale());
    ASSERT_EQ(expectedKeyFrames, animationController_.getKeyFrameStore().at(DUMMY_OBJ_NAME).kQueue.size());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (FLOAT UPDATE) when the keyframe
 * is updated multiple times.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameUpdatesMultipleTimes_ThenFloatTransformationOccursForObject) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float originalScale = 1.0f;
    float desiredScale = 9.0f;
    float targetTime = 3.0f;
    deltaTime = 1.0f;
    float expectedTransformation = originalScale + ((desiredScale - originalScale) * ((deltaTime * 2) / targetTime));
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);
    int expectedKeyFrames = 1;
    obj.setScale(originalScale);
    keyFrame_1.get()->scale.desired = desiredScale;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();
    animationController_.update();

    /* Validation */
    ASSERT_EQ(expectedTransformation, obj.getScale());
    ASSERT_EQ(expectedKeyFrames, animationController_.getKeyFrameStore().at(DUMMY_OBJ_NAME).kQueue.size());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (FLOAT UPDATE) when the keyframe
 * is updated multiple times to completion. When completed, the object should be in the desired state exactly.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKFUpdatesMultTimesComplete_ThenFinalFloatTransformationOccursForObject) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float originalScale = 1.0f;
    float desiredScale = 9.0f;
    float targetTime = 3.0f;
    deltaTime = 1.0f;
    float expectedTransformation = desiredScale;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);

    obj.setScale(originalScale);
    keyFrame_1.get()->scale.desired = desiredScale;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();
    animationController_.update();
    animationController_.update();

    /* Validation */
    ASSERT_EQ(expectedTransformation, obj.getScale());
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (FLOAT UPDATE) when the keyframe
 * is updated with a delta time equal to the target time.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenFloatKeyFrameCompletes_ThenFinalUpdateOccurs) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float originalScale = 1.0f;
    float desiredScale = 9.0f;
    float targetTime = 3.0f;
    deltaTime = targetTime;
    float expectedTransformation = desiredScale;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);

    obj.setScale(originalScale);
    keyFrame_1.get()->scale.desired = desiredScale;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_EQ(expectedTransformation, obj.getScale());
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
}

/**
 * @brief Ensure that a keyframe updates the target object to the desired state, even when the delta time
 * for the update would raise the current time beyond the target time.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenFloatKeyFrameCompletesWithOverflow_ThenFinalUpdateOccurs) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float originalScale = 1.0f;
    float desiredScale = 9.0f;
    float targetTime = 7.0f;
    deltaTime = targetTime;
    float expectedTransformation = desiredScale;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);

    obj.setScale(originalScale);
    keyFrame_1.get()->scale.desired = desiredScale;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_EQ(expectedTransformation, obj.getScale());
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (VECTOR UPDATE).
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameUpdates_ThenVectorTransformationOccursForObject) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    vec3 originalPosition(1.0f, 2.0f, 3.0f);
    vec3 desiredPosition(6.0f, 5.0f, 4.0f);
    float targetTime = 3.0f;
    deltaTime = 1.0f;
    vec3 expectedTransformation = originalPosition +
        ((desiredPosition - originalPosition) * vec3(deltaTime / targetTime));
    int expectedKeyFrames = 1;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);

    obj.setPosition(expectedTransformation);
    keyFrame_1.get()->pos.desired = desiredPosition;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_VEC_EQ(expectedTransformation, obj.getPosition());
    ASSERT_EQ(expectedKeyFrames, animationController_.getKeyFrameStore().at(DUMMY_OBJ_NAME).kQueue.size());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (VECTOR UPDATE) when the keyframe
 * is updated multiple times.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameUpdatesTwice_ThenVectorTransformationOccursForObject) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    vec3 originalPosition(1.0f, 2.0f, 3.0f);
    vec3 desiredPosition(6.0f, 5.0f, 4.0f);
    float targetTime = 3.0f;
    deltaTime = 1.0f;
    vec3 expectedTransformation = originalPosition +
        ((desiredPosition - originalPosition) * vec3((deltaTime * 2) / targetTime));
    int expectedKeyFrames = 1;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);

    obj.setPosition(expectedTransformation);
    keyFrame_1.get()->pos.desired = desiredPosition;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();
    animationController_.update();

    /* Validation */
    ASSERT_VEC_EQ(expectedTransformation, obj.getPosition());
    ASSERT_EQ(expectedKeyFrames, animationController_.getKeyFrameStore().at(DUMMY_OBJ_NAME).kQueue.size());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (VECTOR UPDATE) when the keyframe
 * is updated multiple times to completion. When completed, the object should be in the desired state exactly.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameCompletes_ThenFinalVectorTransformationOccurs) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    vec3 originalPosition(1.0f, 2.0f, 3.0f);
    vec3 desiredPosition(6.0f, 5.0f, 4.0f);
    float targetTime = 3.0f;
    deltaTime = 1.0f;
    vec3 expectedTransformation = desiredPosition;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_POS, targetTime);

    obj.setPosition(originalPosition);
    keyFrame_1.get()->pos.desired = desiredPosition;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();
    animationController_.update();
    animationController_.update();

    /* Validation */
    ASSERT_VEC_EQ(expectedTransformation, obj.getPosition());
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (VECTOR UPDATE) when the keyframe
 * is updated with a delta time equal to the target time.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameCompletesEntirely_ThenFinalVectorTransformationOccurs) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    vec3 originalPosition(1.0f, 2.0f, 3.0f);
    vec3 desiredPosition(6.0f, 5.0f, 4.0f);
    float targetTime = 3.0f;
    deltaTime = 3.0f;
    vec3 expectedTransformation = desiredPosition;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_POS, targetTime);

    obj.setPosition(originalPosition);
    keyFrame_1.get()->pos.desired = desiredPosition;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_VEC_EQ(expectedTransformation, obj.getPosition());
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
}

/**
 * @brief Ensure that a keyframe update actually transforms an object as expected (VECTOR UPDATE) when the keyframe
 * is updated with a delta time greater than the target time. This should result in the object being
 * transformed to the desired state exactly.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameCompletesWithOverflow_ThenFinalVectorTransformationOccurs) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    vec3 originalPosition(1.0f, 2.0f, 3.0f);
    vec3 desiredPosition(6.0f, 5.0f, 4.0f);
    float targetTime = 3.0f;
    deltaTime = 7.0f;
    vec3 expectedTransformation = desiredPosition;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_POS, targetTime);

    obj.setPosition(originalPosition);
    keyFrame_1.get()->pos.desired = desiredPosition;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_VEC_EQ(expectedTransformation, obj.getPosition());
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
}

/**
 * @brief Ensures that multiple keyframes will be updated simultaneously when owned by different objects.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFramesAddedForMultipleObjects_ThenBothObjectsUpdateSimultaneously) {
    /* Preparation */
    auto name_1 = "testObject1";
    auto name_2 = "testObject2";
    TestObject obj_1(name_1);
    TestObject obj_2(name_2);
    vec3 originalPosition_1(1.0f, 2.0f, 3.0f);
    vec3 originalPosition_2(9.0f, 10.f, 11.0f);
    vec3 desiredPosition(6.0f, 5.0f, 4.0f);
    float targetTime = 3.0f;
    deltaTime = 1.0f;
    vec3 expectedTransformation_1 = originalPosition_1 +
        ((desiredPosition - originalPosition_1) * vec3((deltaTime) / targetTime));
    vec3 expectedTransformation_2 = originalPosition_2 +
        ((desiredPosition - originalPosition_2) * vec3((deltaTime) / targetTime));
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_POS, targetTime);
    auto keyFrame_2 = AnimationController::createKeyFrame(UPDATE_POS, targetTime);

    obj_1.setPosition(originalPosition_1);
    obj_2.setPosition(originalPosition_2);

    keyFrame_1.get()->pos.desired = desiredPosition;
    keyFrame_2.get()->pos.desired = desiredPosition;

    animationController_.addKeyFrame(&obj_1, keyFrame_1);
    animationController_.addKeyFrame(&obj_2, keyFrame_2);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_VEC_EQ(expectedTransformation_1, obj_1.getPosition());
    ASSERT_VEC_EQ(expectedTransformation_2, obj_2.getPosition());
    ASSERT_EQ(2, animationController_.getKeyFrameStore().size());
    ASSERT_TRUE(animationController_.getKeyFrameStore().find(name_1) != animationController_.getKeyFrameStore().end());
    ASSERT_TRUE(animationController_.getKeyFrameStore().find(name_2) != animationController_.getKeyFrameStore().end());
    // Both keyframes should still be in the store
    ASSERT_EQ(1, animationController_.getKeyFrameStore().at(name_1).kQueue.size());
    ASSERT_EQ(1, animationController_.getKeyFrameStore().at(name_2).kQueue.size());
}

/**
 * @brief Ensures that when multiple keyframes are present in the keyframe store, but only one of them completes,
 * the incomplete keyframe will be updated and present in the store.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameCompletesForOneObject_ThenOtherObjectRemainsInStore) {
    /* Preparation */
    auto name_1 = "testObject1";
    auto name_2 = "testObject2";
    TestObject obj_1(name_1);
    TestObject obj_2(name_2);
    vec3 originalPosition_1(1.0f, 2.0f, 3.0f);
    vec3 originalPosition_2(9.0f, 10.f, 11.0f);
    vec3 desiredPosition(6.0f, 5.0f, 4.0f);
    float targetTime_1 = 3.0f;
    float targetTime_2 = 5.0f;  // Different target time for the second object
    deltaTime = 4.0f;
    vec3 expectedTransformation_1 = desiredPosition;  // This keyframe should complete
    vec3 expectedTransformation_2 = originalPosition_2 +
        ((desiredPosition - originalPosition_2) * vec3(deltaTime / targetTime_2));
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_POS, targetTime_1);
    auto keyFrame_2 = AnimationController::createKeyFrame(UPDATE_POS, targetTime_2);

    obj_1.setPosition(originalPosition_1);
    obj_2.setPosition(originalPosition_2);

    keyFrame_1.get()->pos.desired = desiredPosition;
    keyFrame_2.get()->pos.desired = desiredPosition;

    animationController_.addKeyFrame(&obj_1, keyFrame_1);
    animationController_.addKeyFrame(&obj_2, keyFrame_2);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_VEC_EQ(expectedTransformation_1, obj_1.getPosition());
    ASSERT_VEC_EQ(expectedTransformation_2, obj_2.getPosition());
    ASSERT_EQ(1, animationController_.getKeyFrameStore().size());
    ASSERT_TRUE(animationController_.getKeyFrameStore().find(name_1) == animationController_.getKeyFrameStore().end());
    ASSERT_TRUE(animationController_.getKeyFrameStore().find(name_2) != animationController_.getKeyFrameStore().end());
    // Both keyframes should still be in the store
    ASSERT_EQ(1, animationController_.getKeyFrameStore().at(name_2).kQueue.size());
}

/**
 * @brief Checks that an associated callback is run when a keyframe completes.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameCompletes_ThenCallBackIsRun) {
    /* Preparation */
    auto passTest = false;
    TestObject obj(DUMMY_OBJ_NAME);
    float targetTime_1 = 3.0f;
    deltaTime = 3.0f;

    auto callback = [&passTest]() {
        passTest = true;
    };
    auto keyFrame_1 = AnimationController::createKeyFrameCb(UPDATE_NONE, callback, targetTime_1);

    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
    // Ensure the callback was run if this was changed
    ASSERT_TRUE(passTest);
}

/**
 * @brief This tests an edge case I ran into in a studious game I made (gameshow). I wanted to add a keyframe for
 * another object dynamically upon a keyframe completing. This resulted in a deadlock due to the animation controller
 * lock being held by the update method. I added some changes that explicitly unlock the animation controller before
 * running callbacks to support this functionality. It's pretty niche, but it's useful sometimes. - Christian
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenCallbackAddsKeyFrame_ThenNoDeadLockOccurs) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float targetTime_1 = 3.0f;
    float targetTime_2 = 5.0f;  // Different target time for the second keyframe
    deltaTime = 3.0f;

    auto keyFrame_2 = AnimationController::createKeyFrame(UPDATE_POS, targetTime_2);
    auto addKfCb = [&keyFrame_2, &obj, this]() {
        animationController_.addKeyFrame(&obj, keyFrame_2);
    };
    auto keyFrame_1 = AnimationController::createKeyFrameCb(UPDATE_NONE, addKfCb, targetTime_1);

    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    // Make sure the first keyframe was removed
    ASSERT_EQ(1, animationController_.getKeyFrameStore().size());
    ASSERT_TRUE(animationController_.getKeyFrameStore().find(DUMMY_OBJ_NAME) !=
        animationController_.getKeyFrameStore().end());
    // The second keyframe should be in the queue
    ASSERT_EQ(1, animationController_.getKeyFrameStore().at(DUMMY_OBJ_NAME).kQueue.size());
    // The second keyframe should be the one we added
    ASSERT_EQ(UPDATE_POS, animationController_.getKeyFrameStore().at(DUMMY_OBJ_NAME).kQueue.front().get()->type);
    ASSERT_EQ(targetTime_2,
        animationController_.getKeyFrameStore().at(DUMMY_OBJ_NAME).kQueue.front().get()->targetTime);
}

/**
 * @brief Tests text-based transformations.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenProcessTextTransformation_ThenTextTransformationOccurs) {
    /* Preparation */
    string desiredText = "Hello, World!";
    /* This is one third of the desired text, which is what we expect after being one-third through the keyframe */
    string expected = "Hell";
    TextObject obj("", vec3(0), 1.0f, testFontPath, 1.0f, 10, 0, 0, TEST_OBJECT_NAME,
        ObjectType::TEXT_OBJECT, &dummyGfxController_);
    float targetTime_1 = 3.0f;
    deltaTime = 1.0f;

    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_TEXT, targetTime_1);
    keyFrame_1.get()->text.desired = desiredText;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    // Make sure the first keyframe was removed
    ASSERT_EQ(1, animationController_.getKeyFrameStore().size());
    ASSERT_NE(animationController_.getKeyFrameStore().find(TEST_OBJECT_NAME),
        animationController_.getKeyFrameStore().end());
    ASSERT_EQ(expected, obj.getMessage());
}

/**
 * @brief Tests text-based transformations on completion
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenTextTransformationCompletes_ThenTextUpdatedToDesired) {
    /* Preparation */
    string desiredText = "Hello, World!";
    TextObject obj("", vec3(0), 1.0f, testFontPath, 1.0f, 10, 0, 0, TEST_OBJECT_NAME,
        ObjectType::TEXT_OBJECT, &dummyGfxController_);
    float targetTime_1 = 3.0f;
    deltaTime = 3.0f;

    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_TEXT, targetTime_1);
    keyFrame_1.get()->text.desired = desiredText;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    // Make sure the first keyframe was removed
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
    ASSERT_EQ(desiredText, obj.getMessage());
}

/**
 * @brief Ensures that keyframes with a zero time do not crash the animation controller, and that
 * the object is updated immediately to the desired state.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenZeroTimeKeyFrameUpdates_ThenObjectUpdatedImmediately) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float originalScale = 1.0f;
    float desiredScale = 9.0f;
    float targetTime = 0.0f;
    deltaTime = 1.0f;
    float expectedTransformation = desiredScale;  // Since target time is 0, we expect the desired value immediately
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime);

    obj.setScale(originalScale);
    keyFrame_1.get()->scale.desired = desiredScale;
    animationController_.addKeyFrame(&obj, keyFrame_1);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_EQ(expectedTransformation, obj.getScale());
}

/**
 * @brief Tests animation keyframe bleed-through behavior. When a keyframe completes with excess time remaining, the
 * next keyframe should process with the remaining time after that keyframe completes. This should result for
 * smoother performance at lower framerates.
 *
 */
TEST_F(GivenAnAnimationControllerReady, WhenKeyFrameTimeOverflows_ThenNextKeyFramesProcesses) {
    /* Preparation */
    TestObject obj(DUMMY_OBJ_NAME);
    float originalScale = 1.0f;
    float desiredScale_1 = 9.0f;
    float desiredScale_2 = 15.0f;
    float desiredScale_3 = 20.0f;
    float targetTime_1 = 3.0f;
    float targetTime_2 = 2.0f;
    float targetTime_3 = 1.0f;
    deltaTime = 6.0f;
    float expectedTransformation = desiredScale_3;
    auto keyFrame_1 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime_1);
    auto keyFrame_2 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime_2);
    auto keyFrame_3 = AnimationController::createKeyFrame(UPDATE_SCALE, targetTime_3);

    obj.setScale(originalScale);
    keyFrame_1.get()->scale.desired = desiredScale_1;
    keyFrame_2.get()->scale.desired = desiredScale_2;
    keyFrame_3.get()->scale.desired = desiredScale_3;
    animationController_.addKeyFrame(&obj, keyFrame_1);
    animationController_.addKeyFrame(&obj, keyFrame_2);
    animationController_.addKeyFrame(&obj, keyFrame_3);

    /* Action */
    animationController_.update();

    /* Validation */
    ASSERT_EQ(expectedTransformation, obj.getScale());
    ASSERT_TRUE(animationController_.getKeyFrameStore().empty());
}
