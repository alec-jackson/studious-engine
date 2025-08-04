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

// Test Fixtures
class GivenPhysicsControllerThreaded: public ::testing::Test {
 protected:
    void SetUp() override {
        // Create a physics controller with 6 threads
        physicsController_ = new PhysicsController(6);
    }
    void TearDown() override {
        delete physicsController_;
    }
    PhysicsController *physicsController_;
    Polygon dummyPoly_;
    DummyGfxController gfxController_;
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
TEST_F(GivenPhysicsControllerThreaded, WhenSceneObjectAdded_ThenObjectPresentInsideController) {
    /* Preparation */
    string testObjectName = "testObject";
    vec3 testObjectPosition = vec3(5);
    bool isKinematic = true;
    bool obeyGravity = false;
    float elasticity = 1.0f;
    int expectedObjects = 1;
    auto testObject = TestObject(testObjectName);
    PhysicsParams params = {
        testObjectPosition,
        isKinematic,
        obeyGravity,
        elasticity
    };

    /* Action */
    physicsController_->addSceneObject(&testObject, params);

    /* Validation */
    auto objectList = physicsController_->getPhysicsObjects();
    ASSERT_EQ(expectedObjects, objectList.size());
    ASSERT_EQ(testObjectName, objectList.front().get()->target->getObjectName());
    ASSERT_EQ(isKinematic, objectList.front().get()->isKinematic);
    ASSERT_EQ(obeyGravity, objectList.front().get()->obeyGravity);
    ASSERT_FLOAT_EQ(elasticity, objectList.front().get()->elasticity);
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