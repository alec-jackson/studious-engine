/**
 * @file SafeQueueTests.cpp
 * @author Christian Galvez
 * @brief Unit tests for the SafeQueue class
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <gtest/gtest.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <SafeQueueTests.hpp>

TEST(GivenSafeQueue, WhenPushCalled_ThenPopReturnsData) {
    // Preparation
    SafeQueue<exampleData> queue;
    exampleData data = {5, "Hello" };

    // Action
    queue.push(data);
    auto result = queue.pop();

    // Validation
    ASSERT_EQ(data.size, result.size);
    ASSERT_EQ(0, strncmp(data.data, result.data, data.size));
}

TEST(GivenSafeQueue, WhenMultipleItemsPushed_MultiplePopsUnblocked) {
    // Preparation
    SafeQueue<exampleData> queue;
    exampleData data = {5, "Hello" };

    // Action
    queue.push(data);
    queue.push(data);
    queue.push(data);
    queue.push(data);
    queue.pop();
    queue.pop();
    queue.pop();
    auto result = queue.pop();

    // Validation
    ASSERT_EQ(data.size, result.size);
    ASSERT_EQ(0, strncmp(data.data, result.data, data.size));
}

TEST(GivenSafeQueue, WhenMultipleItemsPushed_SizeUpdateAccordingly) {
    // Preparation
    SafeQueue<exampleData> queue;
    exampleData data = {5, "Hello" };
    auto expectedSize = 3;

    // Action
    queue.push(data);
    queue.push(data);
    queue.push(data);
    queue.push(data);
    queue.pop();

    // Validation
    ASSERT_EQ(expectedSize, queue.size());
}

TEST(GivenSafeQueue, WhenPopCalledOnEmptyQueue_ThreadWaitsUntilValuePushed) {
    // Preparation
    auto popped = 0;
    auto expectedPopped = 1;
    auto expectedSize = 0;
    SafeQueue<exampleData> queue;
    std::mutex testMutex;
    testMutex.lock();  // Lock the mutex from the get-go
    auto poperation = [&queue, &testMutex, &popped]() {
        queue.pop();
        ++popped;
        // Unlock the test mutex when the pop operation completes
        testMutex.unlock();
    };
    exampleData data = {6, "Hello." };

    // Action
    // Create a new thread running the POPeration
    std::thread qThread(poperation);
    queue.push(data);
    qThread.join();
    // Wait for testMutex unlock from lambda func
    testMutex.lock();

    // Validation
    ASSERT_EQ(expectedPopped, popped);
    ASSERT_EQ(expectedSize, queue.size());
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
