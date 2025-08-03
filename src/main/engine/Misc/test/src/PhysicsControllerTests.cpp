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