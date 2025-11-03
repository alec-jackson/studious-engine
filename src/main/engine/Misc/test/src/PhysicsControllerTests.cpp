/**
 * @file PhysicsControllerTests.cpp
 * @author Christian Galvez
 * @brief Unit tests for the studious physics controller
 * @version 0.1
 * @date 2024-07-07
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <PhysicsControllerTests.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <cstdio>
#include <ColliderObject.hpp>
#include <ModelImport.hpp>
#include <physics.hpp>
#include <TestObject.hpp>

extern double deltaTime;

const char *testObjectName = "testObject";
const char *otherObjectName = "otherObject";
const char *mapObjectName = "mapObject";
float testMassKg = 5.0f;

// Making this a macro to preserve line number in assert
#define ASSERT_VEC_EQ(expected, actual) \
for (uint i = 0; i < (sizeof(actual) / sizeof(float)); ++i) ASSERT_FLOAT_EQ(expected[i], actual[i]);

#define EXPECT_VEC_EQ(expected, actual) \
for (uint i = 0; i < (sizeof(actual) / sizeof(float)); ++i) { \
    printf("float eq in %d\n", i); EXPECT_FLOAT_EQ(expected[i], actual[i]); }

// Test Fixtures
class GivenPhysicsControllerGeneral: public ::testing::Test {
 protected:
    void SetUp() override {
        // Create a physics controller with 6 threads
        physicsController_ = new PhysicsController(6);
    }
    void TearDown() override {
        delete physicsController_;
    }
    PhysicsController *physicsController_;
};

/**
 * @brief Ensures that worker threads will cleanup properly when the physics controller is destroyed.
 */
TEST(GivenPhysicsController, WhenConstructedWithThreads_ThenDestructorClosesThreadsGracefully) {
    /* Preparation */
    auto physicsController = new PhysicsController(6);

    /* Action / Validation */
    // Deleting the physics controller will either crash or hang if broken here...
    delete physicsController;
}

/**
 * @brief Tests adding a scene object to the physics controller.
 *
 */
TEST_F(GivenPhysicsControllerGeneral, WhenSceneObjectAdded_ThenObjectPresentInsideController) {
    /* Preparation */
    bool isKinematic = true;
    bool obeyGravity = false;
    float elasticity = 1.0f;
    float mass = 2.0f;
    int expectedObjects = 1;
    auto testObject = TestObject(testObjectName);
    PhysicsParams params = {
        isKinematic,
        obeyGravity,
        elasticity,
        mass
    };

    /* Action */
    physicsController_->addSceneObject(&testObject, params);

    /* Validation */
    auto objectMap = physicsController_->getPhysicsObjects();
    auto oit = objectMap.find(testObjectName);
    ASSERT_EQ(expectedObjects, objectMap.size());
    ASSERT_NE(objectMap.end(), oit);  // Verify testObjectName exists
    ASSERT_EQ(testObjectName, oit->second.get()->target->objectName());
    ASSERT_EQ(isKinematic, oit->second.get()->isKinematic);
    ASSERT_EQ(obeyGravity, oit->second.get()->obeyGravity);
    ASSERT_FLOAT_EQ(elasticity, oit->second.get()->elasticity);
    ASSERT_FLOAT_EQ(mass, oit->second.get()->mass);
}

/**
 * @brief Ensures removing a scene object from the physics controller actually removes it properly.
 *
 */
TEST_F(GivenPhysicsControllerGeneral, WhenSceneObjectAddedThenRemoved_ThenSceneObjectRemoved) {
    /* Preparation */
    auto testObject = TestObject(testObjectName);
    PhysicsParams params = {
        false,
        false,
        0.0f,
        0.0f
    };
    physicsController_->addSceneObject(&testObject, params);

    /* Action */
    physicsController_->removeSceneObject(testObjectName);

    /* Validation */
    auto objectList = physicsController_->getPhysicsObjects();
    ASSERT_TRUE(objectList.empty());
}

/**
 * @brief Ensures removing an object that doesn't exist does not have any weird side effects.
 *
 */
TEST_F(GivenPhysicsControllerGeneral, WhenUnknownSceneObjectRemoved_ThenOtherObjectsUnaffected) {
    /* Preparation */
    string unknownName = "whatever";
    auto testObject = TestObject(testObjectName);
    int expectedObjects = 1;
    PhysicsParams params = {
        false,
        false,
        0.0f,
        0.0f
    };
    physicsController_->addSceneObject(&testObject, params);

    /* Action */
    physicsController_->removeSceneObject(unknownName);

    /* Validation */
    auto objectList = physicsController_->getPhysicsObjects();
    auto oit = objectList.find(testObjectName);
    ASSERT_EQ(expectedObjects, objectList.size());
    ASSERT_EQ(testObjectName, oit->second.get()->target->objectName());
}

/**
 * @brief Ensures getting a physics object works as expected.
 *
 */
TEST_F(GivenPhysicsControllerGeneral, WhenSceneObjectAdded_ThenGetPhysicsObjectReturnsIt) {
    /* Preparation */
    auto testObject = TestObject(testObjectName);
    PhysicsParams params = {
        false,
        false,
        0.0f,
        0.0f
    };
    physicsController_->addSceneObject(&testObject, params);

    /* Action */
    auto physObj = physicsController_->getPhysicsObject(testObjectName);

    /* Validation */
    ASSERT_GT(physObj.use_count(), 0);  // use_count is used to determine if pointer is active
    ASSERT_EQ(testObjectName, physObj.get()->target->objectName());
}

/**
 * @brief Ensures getting a physics object that does not exist returns an invalid smart pointer.
 *
 */
TEST_F(GivenPhysicsControllerGeneral, WhenNoPhysObjectsPresent_ThenInvalidPointerReturned) {
    /* Preparation */
    /* Action */
    auto physObj = physicsController_->getPhysicsObject(testObjectName);

    /* Validation */
    ASSERT_EQ(physObj.use_count(), 0);  // use_count is used to determine if pointer is active
}

/* PHYSICS POSITION PIPELINE TESTS */

class GivenPhysicsControllerPositionPipeline: public ::testing::Test {
 protected:
    void SetUp() override {
        // Create a physics controller with 6 threads
        physicsController_ = std::make_unique<PhysicsController>(6);
        // Create a sample test object and add it to the physics controller
        testObject_ = std::make_unique<TestObject>(testObjectName);
        testObject_.get()->setPosition(vec3(0));
        PhysicsParams params = {
            .isKinematic = false,
            .obeyGravity = false,
            .elasticity = 0.0f,
            .mass = testMassKg
        };
        physicsController_->addSceneObject(testObject_.get(), params);
    }
    std::unique_ptr<PhysicsController> physicsController_;
    std::unique_ptr<TestObject> testObject_;
};

/**
 * @brief Ensures that setting the position in the physics controller updates the target object's position.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenPositionUpdateCalled_ThenPositionUpdated) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 startingPosition = vec3(1.0f, 0.0f, 0.0f);
    vec3 expectedPosition = vec3(5.0f, 4.0f, 3.0f);
    testObject_->setPosition(startingPosition);
    physicsController_->setPosition(testObjectName, expectedPosition);
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

/**
 * @brief Ensures that setting the velocity and calling update() updates the position as
 * expected for the amount of time passed.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenVelocityUpdateCalled_ThenPositionUpdated) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 startingPosition = vec3(1.0f, 0.0f, 0.0f);
    vec3 targetVelocity = vec3(4.0f, 4.0f, 3.0f);
    vec3 expectedPosition = vec3(5.0f, 4.0f, 3.0f);
    testObject_->setPosition(startingPosition);
    /* The physics object needs to be updated explicitly because the position
     * is set when the object is inserted. Setting the object's position here
     * directly via the SceneObject setter is meaningless...
     */
    physicsController_->setPosition(testObjectName, startingPosition);
    physicsController_->setVelocity(testObjectName, targetVelocity);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

/**
 * @brief Ensures that setting the acceleration and calling update() updates the position as
 * expected for the amount of time passed.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenAccelerationUpdateCalled_ThenPositionUpdated) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 startingPosition = vec3(1.0f, 0.0f, 0.0f);
    vec3 targetAcceleration = vec3(1.0f, 1.0f, 1.0f);
    vec3 expectedPosition = vec3(1.5f, 0.5f, 0.5f);
    testObject_->setPosition(startingPosition);
    /* The physics object needs to be updated explicitly because the position
     * is set when the object is inserted. Setting the object's position here
     * directly via the SceneObject setter is meaningless...
     */
    physicsController_->setPosition(testObjectName, startingPosition);
    physicsController_->setAcceleration(testObjectName, targetAcceleration);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

/**
 * @brief Ensures that setting the acceleration and calling update() updates the position as
 * expected for the amount of time accumulated after each update call.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenAccelerationUpdateCalledTwice_ThenPositionUpdated) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 startingPosition = vec3(1.0f, 0.0f, 0.0f);
    vec3 targetAcceleration = vec3(1.0f, 1.0f, 1.0f);
    vec3 expectedPosition_2 = vec3(3.0f, 2.0f, 2.0f);
    testObject_->setPosition(startingPosition);
    physicsController_->setPosition(testObjectName, startingPosition);
    physicsController_->setAcceleration(testObjectName, targetAcceleration);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition_2, testObject_->getPosition());
}

/**
 * @brief Validates overall physics calculations for position with complex values for position, velocity
 * and acceleration.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenComplexAccelerationVelPosUpdateCalled_ThenPositionAccurate) {
    /* Preparation */
    deltaTime = 5.67f;
    vec3 startingPosition = vec3(65.4f, 21.0f, 0.9f);
    vec3 velocity = vec3(5.0f, 0.0f, 3.2f);
    vec3 acceleration = vec3(5.7f, -9.81f, 91.0f);
    vec3 expectedPosition = vec3(185.37436f, -136.69035f, 1481.819f);
    testObject_->setPosition(startingPosition);
    physicsController_->setPosition(testObjectName, startingPosition);
    physicsController_->setVelocity(testObjectName, velocity);
    physicsController_->setAcceleration(testObjectName, acceleration);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

/**
 * @brief Ensures that changing an object's velocity after time has accumulated (after update has been called at
 * least once) results in expected object transformations from subsequent update calls.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenUpdateCalledAfterVelocityChanges_ThenPositionTimeFlush) {
    /* Preparation */
    deltaTime = 1.0f;
    auto physicsObject = physicsController_->getPhysicsObject(testObjectName);
    // Going to keep calculations 1 dimensional so it's easier to follow :)
    vec3 startingPosition = vec3(1.0f, 0.0f, 0.0f);
    vec3 velocity = vec3(1.0f, 0.0f, 0.0f);
    vec3 acceleration = vec3(1.0f, 0.0f, 0.0f);
    vec3 expectedPosition_1 = vec3(5.0f, 0.0f, 0.0f);
    vec3 expectedPosition_2 = vec3(6.5f, 0.0f, 0.0f);
    testObject_->setPosition(startingPosition);
    physicsController_->setPosition(testObjectName, startingPosition);
    physicsController_->setVelocity(testObjectName, velocity);
    physicsController_->setAcceleration(testObjectName, acceleration);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    // Run update twice - this basically sets t = 2 seconds
    physicsController_->update();
    physicsController_->update();

    // The position here should be 5...
    ASSERT_VEC_EQ(expectedPosition_1, testObject_->getPosition());

    // The physics object itself should still hold the original reference pos
    ASSERT_VEC_EQ(startingPosition, physicsObject->position);

    // We're going to reset the velocity, which will reset the running time counter...
    physicsController_->setVelocity(testObjectName, velocity);

    // ... and also set the current position as the new reference position
    ASSERT_VEC_EQ(expectedPosition_1, physicsObject->position);

    /* Action */
    // Run update again - should calculate with t = 1 second now...
    physicsController_->update();

    /* Validation */
    // Position is 6.5f now because t = 1 second instead of 3.
    ASSERT_VEC_EQ(expectedPosition_2, testObject_->getPosition());
    // Without the time reset & position flush, the last update call would set the
    // position to 8.5, because t = 3 and pos = 1. Now it's t = 1 but pos = 5.
}

/**
 * @brief Ensures that changing an object's acceleration after time has accumulated (after update has been called at
 * least once) results in expected object transformations from subsequent update calls.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenUpdateCalledAfterAccelerationChanges_ThenPositionTimeFlush) {
    /* Preparation */
    deltaTime = 1.0f;
    auto physicsObject = physicsController_->getPhysicsObject(testObjectName);
    // Going to keep calculations 1 dimensional so it's easier to follow :)
    vec3 startingPosition = vec3(1.0f, 0.0f, 0.0f);
    vec3 velocity = vec3(1.0f, 0.0f, 0.0f);
    vec3 acceleration = vec3(1.0f, 0.0f, 0.0f);
    vec3 expectedPosition_1 = vec3(5.0f, 0.0f, 0.0f);
    vec3 expectedPosition_2 = vec3(8.5f, 0.0f, 0.0f);
    testObject_->setPosition(startingPosition);
    physicsController_->setPosition(testObjectName, startingPosition);
    physicsController_->setVelocity(testObjectName, velocity);
    physicsController_->setAcceleration(testObjectName, acceleration);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    // Run update twice - this basically sets t = 2 seconds
    physicsController_->update();
    physicsController_->update();

    // The position here should be 5...
    ASSERT_VEC_EQ(expectedPosition_1, testObject_->getPosition());

    // The physics object itself should still hold the original reference pos
    ASSERT_VEC_EQ(startingPosition, physicsObject->position);

    // We're going to reset the acceleration, which will reset the running time counter...
    physicsController_->setAcceleration(testObjectName, acceleration);

    // ... and also set the current position as the new reference position
    ASSERT_VEC_EQ(expectedPosition_1, physicsObject->position);

    /* Action */
    // Run update again - should calculate with t = 1 second now...
    physicsController_->update();

    /* Validation */
    /*
    Setting the acceleration above does some interesting stuff. We "flush" the position
    and velocity values using acceleration/velocity to bake the old runningTime variable
    into the new values. This is what this calculation looks like:

    ========== BEFORE FLUSH ==================
    acceleration = 1.0
    velocity = 1.0
    position = 1.0
    runningTime = 2.0

    ========== AFTER FLUSH (IN ORDER) ========
                1.    2
    position =  _ (a)t  + (v)t + position
                2

                     2
    -> 0.5 (1.0)(2.0)  + (1.0)(2.0) + 1.0

    -> 2.0 + 2.0 + 1.0 = 5.0

    velocity = (a)t + v -> (1.0)(2.0) + (1.0) = 3.0

    acceleration = 1.0 (nothing changes :)

    runningTime -> 0.0 (reset)

    The idea behind this is that the momentum from the acceleration is PRESERVED in the velocity,
    so the object continues to travel as expected. This avoids a jittering effect when objects have
    their position/velocity/acceleration attributes changed, or when a new force is applied to an
    object.

    ========== UPDATE ==========

    When update is called, we calculate the new position using the updated flushed values.

    position = 5.0
    velocity = 3.0
    acceleration = 1.0
    runningTime = 1.0

                     2
    -> 0.5 (1.0)(1.0)  + (3.0)(1.0) + 5.0 = 8.5

    Notice this is the same expected position if we had not done the acceleration flush!
                           2
    p(3.0) = 0.5 (1.0)(3.0)  + (1.0)(3.0) + 1.0 = 8.5
    */
    ASSERT_VEC_EQ(expectedPosition_2, testObject_->getPosition());
}

/**
 * @brief Validates applyForce functionality. This basically just adds acceleration but consider's
 * an object's mass (F = ma).
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenUpdateAfterApplyForce_ThenPositionUpdatesAsExpected) {
    /* Preparation */
    deltaTime = 1.0f;
    auto physicsObject = physicsController_->getPhysicsObject(testObjectName);
    vec3 startingPosition = vec3(1.0f, 10.0f, 7.0f);
    vec3 force = vec3(5.0f, 3.0f, 6.0f);
    vec3 expectedPosition = vec3(1.5f, 10.3f, 7.6f);
    testObject_->setPosition(startingPosition);
    physicsController_->applyForce(testObjectName, force);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

/**
 * @brief Validates applyInstantForce functionality.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenUpdateAfterApplyInstantForce_ThenPositionUpdatedAsExpected) {
    /* Preparation */
    deltaTime = 3.0f;
    auto physicsObject = physicsController_->getPhysicsObject(testObjectName);
    vec3 startingPosition = vec3(0.0f, 0.0f, 0.0f);
    vec3 force = vec3(5.0f, 3.0f, 6.0f);
    vec3 expectedPosition = vec3(4.5f, 2.7f, 5.4f);
    testObject_->setPosition(startingPosition);
    physicsController_->applyInstantForce(testObjectName, force);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

/**
 * @brief Validates applyInstantForce time capping.
 */
TEST_F(GivenPhysicsControllerPositionPipeline, WhenUpdateAfterApplyInstantForceTooLong_ThenPositionUpdatedWithCapping) {
    /* Preparation */
    deltaTime = 900.0f;
    auto physicsObject = physicsController_->getPhysicsObject(testObjectName);
    vec3 startingPosition = vec3(0.0f, 0.0f, 0.0f);
    vec3 force = vec3(5.0f, 3.0f, 6.0f);
    vec3 expectedPosition = ((vec3(0.5f) * force) / testMassKg) * (MAX_PHYSICS_UPDATE_TIME * MAX_PHYSICS_UPDATE_TIME);
    testObject_->setPosition(startingPosition);
    physicsController_->applyInstantForce(testObjectName, force);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

class GivenTwoKinematicObjects: public GivenPhysicsControllerPositionPipeline {
 protected:
    void SetUp() override {
        physicsController_ = std::make_unique<PhysicsController>(6);
        // Create the polygons for the test objects
        basicModel_ = std::make_shared<Polygon>();
        vector<float> bmVertices = {
            {  // Dummy vertex points here - just want offset to be 1 and center 0
                -1.0f, -1.0f, -1.0f,  // vertex 1
                1.0f, 1.0f, 1.0f,  // vertex 2
            }
        };
        basicModel_->modelMap["to"] = std::make_shared<Model>(bmVertices.size() / 3, bmVertices);

        otherObject_ = std::make_unique<TestObject>(basicModel_, otherObjectName);
        testObject_ = std::make_unique<TestObject>(basicModel_, testObjectName);
        otherObject_->createCollider();
        testObject_->createCollider();

        // Use the same generic params for each object
        PhysicsParams params = {
            .isKinematic = true,
            .obeyGravity = false,
            .elasticity = 0.0f,
            .mass = testMassKg
        };
        physicsController_->addSceneObject(testObject_.get(), params);
        physicsController_->addSceneObject(otherObject_.get(), params);
    }
    void TearDown() override {
        GivenPhysicsControllerPositionPipeline::TearDown();
    }
    std::unique_ptr<TestObject> otherObject_;
    std::shared_ptr<Polygon> basicModel_;
    inline static float basicModelOffset_ = 1.0f;
};

TEST_F(GivenTwoKinematicObjects, WhenObjectsCollide_ThenVelocitiesUpdatedAsExpected) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 firstObjectVelocity = vec3(1.0f, 0.0f, 0.0f);
    vec3 firstObjectPosition = vec3(0.0f, 0.0f, 0.0f);
    // This will place the second object so that its collider is 0.5 units away from the first object's collider
    vec3 secondObjectPosition = firstObjectPosition + vec3(basicModelOffset_ * 2 + 0.5f, 0.0f, 0.0f);
    physicsController_->setPosition(testObjectName, firstObjectPosition);
    physicsController_->setPosition(otherObjectName, secondObjectPosition);

    // Move the first object into the second object
    physicsController_->setVelocity(testObjectName, firstObjectVelocity);

    // Expected final velocities
    float m1 = testMassKg;
    float m2 = testMassKg;
    vec3 v1 = firstObjectVelocity;
    vec3 v2 = vec3(0.0f);
    vec3 expectedV1f = (((m1 - m2) / (m1 + m2)) * v1) + (((2.0f * m2) / (m1 + m2)) * v2);
    vec3 expectedV2f = (((2.0f * m1) / (m1 + m2)) * v1) - (((m1 - m2) / (m1 + m2)) * v2);

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should be stationary
    auto po = physicsController_->getPhysicsObject(testObjectName);
    vec3 actualV1f = physicsController_->getPhysicsObject(testObjectName)->velocity;
    vec3 actualV2f = physicsController_->getPhysicsObject(otherObjectName)->velocity;
    EXPECT_VEC_EQ(expectedV1f, actualV1f);
    EXPECT_VEC_EQ(expectedV2f, actualV2f);
}

TEST_F(GivenTwoKinematicObjects, WhenObjectsCollide_ThenObjectsMovedToEdgePoint) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 firstObjectVelocity = vec3(1.0f, 0.0f, 0.0f);
    vec3 firstObjectPosition = vec3(0.0f, 0.0f, 0.0f);
    // This will place the second object so that its collider is 0.5 units away from the first object's collider
    vec3 secondObjectPosition = firstObjectPosition + vec3(basicModelOffset_ * 2 + 0.5f, 0.0f, 0.0f);
    physicsController_->setPosition(testObjectName, firstObjectPosition);
    physicsController_->setPosition(otherObjectName, secondObjectPosition);

    // Move the first object into the second object
    physicsController_->setVelocity(testObjectName, firstObjectVelocity);

    // Expected final positions
    vec3 expectedFirstFinalPos = vec3(0.75f, 0.0f, 0.0f);
    vec3 expectedSecondFinalPos = vec3(2.75f, 0.0f, 0.0f);

    // The objects are 0.5 units inside of each other.
    // The first object should be moved 0.25 units to the left, and
    // the second object should be moved 0.25 units to the right. This should
    // clip the objects right next to each other.

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should have a different velocity
    vec3 actualFFP = physicsController_->getPhysicsObject(testObjectName)->position;
    vec3 actualSFP = physicsController_->getPhysicsObject(otherObjectName)->position;
    EXPECT_VEC_EQ(expectedFirstFinalPos, actualFFP);
    EXPECT_VEC_EQ(expectedSecondFinalPos, actualSFP);

    // Ensure that the objects are NO LONGER colliding after clipping to the edge point
    auto isColl = testObject_->getCollider()->getCollision(otherObject_->getCollider());
    ASSERT_NE(ALL_MATCH, isColl);
}

class GivenKinematicAndNonKinematicObject: public GivenPhysicsControllerPositionPipeline {
 protected:
    void SetUp() override {
        physicsController_ = std::make_unique<PhysicsController>(6);
        // Create the polygons for the test objects
        basicModel_ = std::make_shared<Polygon>();

        vector<float> bmVertices = {
            // Dummy vertex points here - just want offset to be 1 and center 0
            -1.0f, -1.0f, -1.0f,  // vertex 1
            1.0f, 1.0f, 1.0f  // vertex 2
        };
        // This represents a flat X-Z plane that would be used as a map.
        vector<float> mapVertices = {
            -1.0f, 0.0f, -1.0f,
            1.0f, 0.0f, 1.0f
        };
        basicModel_->modelMap["to"] = std::make_shared<Model>(bmVertices.size() / 3, bmVertices);
        mapModel_ = std::make_shared<Polygon>();
        mapModel_->modelMap["mo"] = std::make_shared<Model>(mapVertices.size() / 3, mapVertices);

        mapObject_ = std::make_unique<TestObject>(mapModel_, mapObjectName);
        mapObject_->setScale(10.0f);  // Make the map decently large
        testObject_ = std::make_unique<TestObject>(basicModel_, testObjectName);
        testObject_->createCollider();
        mapObject_->createCollider();

        // Use the same generic params for each object
        PhysicsParams kinPar = {
            .isKinematic = true,
            .obeyGravity = false,
            .elasticity = 0.0f,
            .mass = testMassKg
        };
        PhysicsParams nonKinPar = {
            .isKinematic = false,
            .obeyGravity = false,
            .elasticity = 0.0f,
            .mass = testMassKg
        };
        physicsController_->addSceneObject(testObject_.get(), kinPar);
        physicsController_->addSceneObject(mapObject_.get(), nonKinPar);
    }
    std::unique_ptr<TestObject> mapObject_;
    std::shared_ptr<Polygon> basicModel_;
    std::shared_ptr<Polygon> mapModel_;
    inline static float basicModelOffset_ = 1.0f;
};

TEST_F(GivenKinematicAndNonKinematicObject, WhenKinematicCollidesNoPassthrough_ThenObjectClipsToExpectedLocation) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 playerVel = vec3(0.0, -1.5f, 0.0f);
    vec3 playerPos = vec3(0.0f, 2.0f, 0.0f);
    vec3 mapPos = vec3(0.0f, 0.0f, 0.0f);
    // Because of flushing, set velocity first
    physicsController_->setVelocity(testObjectName, playerVel);
    physicsController_->setPosition(testObjectName, playerPos);
    physicsController_->setPosition(mapObjectName, mapPos);


    // Expected final positions
    vec3 expectedPlayerFinalPos = vec3(0.0f, 1.0f, 0.0f);
    vec3 expectedMapFinalPos = vec3(0.0f, 0.0f, 0.0f);

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should have a different velocity
    vec3 actualPlayerFinalPos = testObject_->getPosition();
    vec3 actualMapFinalPos = mapObject_->getPosition();
    EXPECT_VEC_EQ(expectedPlayerFinalPos, actualPlayerFinalPos);
    EXPECT_VEC_EQ(expectedMapFinalPos, actualMapFinalPos);

    // Ensure that the objects are NO LONGER colliding after clipping to the edge point
    auto isColl = testObject_->getCollider()->getCollision(mapObject_->getCollider());
    ASSERT_NE(ALL_MATCH, isColl);
}

TEST_F(GivenKinematicAndNonKinematicObject, WhenKinematicCollides_ThenObjectClipsToExpectedLocation) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 playerVel = vec3(0.0, -2.5f, 0.0f);
    vec3 playerPos = vec3(0.0f, 2.0f, 0.0f);
    vec3 mapPos = vec3(0.0f, 0.0f, 0.0f);
    // Because of flushing, set velocity first
    physicsController_->setVelocity(testObjectName, playerVel);
    physicsController_->setPosition(testObjectName, playerPos);
    physicsController_->setPosition(mapObjectName, mapPos);


    // Expected final positions
    vec3 expectedPlayerFinalPos = vec3(0.0f, 1.0f, 0.0f);
    vec3 expectedMapFinalPos = vec3(0.0f, 0.0f, 0.0f);

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should have a different velocity
    vec3 actualPlayerFinalPos = testObject_->getPosition();
    vec3 actualMapFinalPos = mapObject_->getPosition();
    EXPECT_VEC_EQ(expectedPlayerFinalPos, actualPlayerFinalPos);
    EXPECT_VEC_EQ(expectedMapFinalPos, actualMapFinalPos);

    // Ensure that the objects are NO LONGER colliding after clipping to the edge point
    auto isColl = testObject_->getCollider()->getCollision(mapObject_->getCollider());
    ASSERT_NE(ALL_MATCH, isColl);
}

TEST_F(GivenKinematicAndNonKinematicObject, WhenKinematicCollidesWithCorner_ThenObjectClipsToExpectedLocation) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 playerVel = vec3(0.0, -2.5f, 0.0f);
    vec3 playerPos = vec3(9.5f, 2.0f, 9.5f);
    vec3 mapPos = vec3(0.0f, 0.0f, 0.0f);
    // Because of flushing, set velocity first
    physicsController_->setVelocity(testObjectName, playerVel);
    physicsController_->setPosition(testObjectName, playerPos);
    physicsController_->setPosition(mapObjectName, mapPos);

    // Expected final positions
    vec3 expectedPlayerFinalPos = vec3(9.5f, 1.0f, 9.5f);
    vec3 expectedMapFinalPos = vec3(0.0f, 0.0f, 0.0f);

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should have a different velocity
    vec3 actualPlayerFinalPos = testObject_->getPosition();
    vec3 actualMapFinalPos = mapObject_->getPosition();
    EXPECT_VEC_EQ(expectedPlayerFinalPos, actualPlayerFinalPos);
    EXPECT_VEC_EQ(expectedMapFinalPos, actualMapFinalPos);

    // Ensure that the objects are NO LONGER colliding after clipping to the edge point
    auto isColl = testObject_->getCollider()->getCollision(mapObject_->getCollider());
    ASSERT_NE(ALL_MATCH, isColl);
}

TEST_F(GivenKinematicAndNonKinematicObject, WhenKinematicCollidesWithCornerAndFalls_ThenObjectDropsWhenExpected) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 playerVel = vec3(0.0f, -2.5f, 1.0f);
    vec3 playerPos = vec3(9.5f, 2.0f, 9.5f);
    vec3 mapPos = vec3(0.0f, 0.0f, 0.0f);
    // Because of flushing, set velocity first
    physicsController_->setVelocity(testObjectName, playerVel);
    physicsController_->setPosition(testObjectName, playerPos);
    physicsController_->setPosition(mapObjectName, mapPos);

    // Expected player final position
    vec3 epfp_update1 = vec3(9.5f, 1.0f, 10.5f);
    // The object "rolls" off the corner of the surface after the second update
    vec3 epfp_update2 = vec3(9.5f, -1.5f, 11.5f);
    vec3 expectedMapFinalPos = vec3(0.0f, 0.0f, 0.0f);

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should have a different velocity
    vec3 afp = testObject_->getPosition();
    vec3 amfp = mapObject_->getPosition();
    EXPECT_VEC_EQ(epfp_update1, afp);
    EXPECT_VEC_EQ(expectedMapFinalPos, amfp);

    /* Action 2 */
    physicsController_->update();

    /* Validation 2 */
    afp = testObject_->getPosition();
    amfp = mapObject_->getPosition();
    EXPECT_VEC_EQ(epfp_update2, afp);
    EXPECT_VEC_EQ(expectedMapFinalPos, amfp);

    // Ensure that the objects are NO LONGER colliding after clipping to the edge point
    auto isColl = testObject_->getCollider()->getCollision(mapObject_->getCollider());
    ASSERT_NE(ALL_MATCH, isColl);
}

TEST_F(GivenKinematicAndNonKinematicObject, WhenKinematicCollidesNoPTNegSign_ThenObjectClipsToExpectedLocation) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 playerVel = vec3(0.0, 1.5f, 0.0f);
    vec3 playerPos = vec3(0.0f, -2.0f, 0.0f);
    vec3 mapPos = vec3(0.0f, 0.0f, 0.0f);
    // Because of flushing, set velocity first
    physicsController_->setVelocity(testObjectName, playerVel);
    physicsController_->setPosition(testObjectName, playerPos);
    physicsController_->setPosition(mapObjectName, mapPos);


    // Expected final positions
    vec3 expectedPlayerFinalPos = vec3(0.0f, -1.0f, 0.0f);
    vec3 expectedMapFinalPos = vec3(0.0f, 0.0f, 0.0f);

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should have a different velocity
    vec3 actualPlayerFinalPos = testObject_->getPosition();
    vec3 actualMapFinalPos = mapObject_->getPosition();
    EXPECT_VEC_EQ(expectedPlayerFinalPos, actualPlayerFinalPos);
    EXPECT_VEC_EQ(expectedMapFinalPos, actualMapFinalPos);

    // Ensure that the objects are NO LONGER colliding after clipping to the edge point
    auto isColl = testObject_->getCollider()->getCollision(mapObject_->getCollider());
    ASSERT_NE(ALL_MATCH, isColl);
}

TEST_F(GivenKinematicAndNonKinematicObject, WhenKinematicCollidesWithCornerAndRises_ThenObjectRaisesWhenExpected) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 playerVel = vec3(0.0f, 2.5f, 1.0f);
    vec3 playerPos = vec3(9.5f, -2.0f, 9.5f);
    vec3 mapPos = vec3(0.0f, 0.0f, 0.0f);
    // Because of flushing, set velocity first
    physicsController_->setVelocity(testObjectName, playerVel);
    physicsController_->setPosition(testObjectName, playerPos);
    physicsController_->setPosition(mapObjectName, mapPos);

    // Expected player final position
    vec3 epfp_update1 = vec3(9.5f, -1.0f, 10.5f);
    // The object "rolls" off the corner of the surface after the second update
    vec3 epfp_update2 = vec3(9.5f, 1.5f, 11.5f);
    vec3 expectedMapFinalPos = vec3(0.0f, 0.0f, 0.0f);

    /* Action */
    physicsController_->update();

    /* Validation */
    // The second object should be moving, and the first should have a different velocity
    vec3 afp = testObject_->getPosition();
    vec3 amfp = mapObject_->getPosition();
    EXPECT_VEC_EQ(epfp_update1, afp);
    EXPECT_VEC_EQ(expectedMapFinalPos, amfp);

    /* Action 2 */
    physicsController_->update();

    /* Validation 2 */
    afp = testObject_->getPosition();
    amfp = mapObject_->getPosition();
    EXPECT_VEC_EQ(epfp_update2, afp);
    EXPECT_VEC_EQ(expectedMapFinalPos, amfp);

    // Ensure that the objects are NO LONGER colliding after clipping to the edge point
    auto isColl = testObject_->getCollider()->getCollision(mapObject_->getCollider());
    ASSERT_NE(ALL_MATCH, isColl);
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
