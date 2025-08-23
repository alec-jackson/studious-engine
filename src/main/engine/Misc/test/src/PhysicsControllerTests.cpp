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
#include <gtest/gtest.h>
#include <iostream>
#include <TestObject.hpp>
#include <PhysicsControllerTests.hpp>
#include <memory>

extern double deltaTime;

string testObjectName = "testObject";

// Helper function to check for vec3 float equality
template<typename T>
void ASSERT_VEC_EQ(const T &expected, const T &actual) {
    uint containerSize = sizeof(T) / sizeof(float);
    for (uint i = 0; i < containerSize; ++i) {
        ASSERT_FLOAT_EQ(expected[i], actual[i]);
    }
}

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
    ASSERT_EQ(testObjectName, oit->second.get()->target->getObjectName());
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
    ASSERT_EQ(testObjectName, oit->second.get()->target->getObjectName());
}

/**
 * @brief Ensures getting a physics object works as expected.
 *
 */
TEST_F(GivenPhysicsControllerGeneral, WhenSceneObjectAdded_GetPhysicsObjectReturnsIt) {
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
    ASSERT_TRUE(physObj.use_count() > 0);  // use_count is used to determine if pointer is active
    ASSERT_EQ(testObjectName, physObj.get()->target->getObjectName());
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
    ASSERT_TRUE(physObj.use_count() == 0);  // use_count is used to determine if pointer is active
}

/* PHYSICS POSITION PIPELINE TESTS */

class GivenPhysicsControllerPositionPipeline: public ::testing::Test {
 protected:
    void SetUp() override {
        // Create a physics controller with 6 threads
        physicsController_ = new PhysicsController(6);
        // Create a sample test object and add it to the physics controller
        testObject_ = std::make_unique<TestObject>(testObjectName);
        testObject_.get()->setPosition(vec3(0));
        PhysicsParams params;
        params.elasticity = 0.0f;
        params.isKinematic = false;
        params.mass = 5.0f;
        params.obeyGravity = false;
        physicsController_->addSceneObject(testObject_.get(), {});
    }
    void TearDown() override {
        delete physicsController_;
    }
    PhysicsController *physicsController_;
    std::unique_ptr<TestObject> testObject_;
};

TEST_F(GivenPhysicsControllerPositionPipeline, WhenPositionUpdateCalled_ThenPositionUpdated) {
    /* Preparation */
    deltaTime = 1.0f;
    vec3 startingPosition = vec3(1.0f, 0.0f, 0.0f);
    vec3 expectedPosition = vec3(5.0f, 4.0f, 3.0f);
    testObject_->setPosition(startingPosition);
    physicsController_->setPosition(testObjectName, expectedPosition);
    ASSERT_VEC_EQ(startingPosition, testObject_->getPosition());

    /* Action */
    physicsController_->update();

    /* Validation */
    ASSERT_VEC_EQ(expectedPosition, testObject_->getPosition());
}

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

TEST_F(GivenPhysicsControllerPositionPipeline, WhenUpdateCalledThenVelocityChanges_ThenPositionTimeFlush) {
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
