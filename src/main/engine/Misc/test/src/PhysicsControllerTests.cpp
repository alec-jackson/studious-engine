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

string testObjectName = "testObject";

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
    auto objectMap = physicsController_->getPhysicsObjects();
    auto oit = objectMap.find(testObjectName);
    ASSERT_EQ(expectedObjects, objectMap.size());
    ASSERT_NE(objectMap.end(), oit);  // Verify testObjectName exists
    ASSERT_EQ(testObjectName, oit->second.get()->target->getObjectName());
    ASSERT_EQ(isKinematic, oit->second.get()->isKinematic);
    ASSERT_EQ(obeyGravity, oit->second.get()->obeyGravity);
    ASSERT_FLOAT_EQ(elasticity, oit->second.get()->elasticity);
}

/**
 * @brief Ensures removing a scene object from the physics controller actually removes it properly.
 *
 */
TEST_F(GivenPhysicsControllerGeneral, WhenSceneObjectAddedThenRemoved_ThenSceneObjectRemoved) {
    /* Preparation */
    auto testObject = TestObject(testObjectName);
    PhysicsParams params = {
        vec3(0),
        false,
        false,
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
        vec3(0),
        false,
        false,
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
        vec3(0),
        false,
        false,
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
    }
    void TearDown() override {
        delete physicsController_;
    }
    PhysicsController *physicsController_;
    std::unique_ptr<TestObject> testObject_;
};

TEST_F(GivenPhysicsControllerPositionPipeline, WhenPositionUpdateCalled_ThenPositionUpdated) {
    /* Preparation */

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
