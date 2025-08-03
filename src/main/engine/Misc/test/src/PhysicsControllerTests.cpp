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
 * @brief Ensure that we can start the physics scheduler, stop it, and proceed normally.
 */
TEST_F(GivenPhysicsControllerThreaded, WhenShutdownSent_ThenSchedulerStops) {
    printf("Entered test\n");
    // Need a dummy polygon
    TestObject items[2] = {
        TestObject("TestObject0"),
        TestObject("TestObject1")
    };
    printf("Creating gameobjects\n");
    auto setShutdown = [this]() {
        printf("Entered shutdown thread\n");
        usleep(5000);  // Sleep for 5ms
        printf("Running shutdown now...\n");
        physicsController_->shutdown();
    };

    // Add 2 gameObjects
    for (int i = 0; i < 2; i++) {
        printf("Adding gameobject %d\n", i);
        PhysicsParams params = { {0.0f, 0.0f, 0.0f}, true, true, 1.0f };
        physicsController_->addSceneObject(&items[i], params);
    }
    // Send the sleep signal in 2 seconds and start the scheduler
    auto shutdownThread = std::thread(setShutdown);
    physicsController_->physicsScheduler();
    shutdownThread.join();
    ASSERT_EQ(true, physicsController_->hasShutdown());
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