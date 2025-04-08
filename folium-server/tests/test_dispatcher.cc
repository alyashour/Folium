// test_dispatcher.cc
#define DISPATCHER_TEST  // Ensure test hooks in dispatcher.cpp are enabled
#include "dispatcher.h"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <string>
#include <iostream>

// Provide stub implementations for Core functions used by the dispatcher.
namespace Core {
    // Stub for createNote: prints an identifiable output.
    void createNote(int noteID, int userID, const std::string &noteData) {
        std::cout << "[TestCore] createNote called: noteID=" << noteID
                  << ", userID=" << userID << ", noteData=" << noteData << std::endl;
    }
    // Stub for editNote: prints an identifiable output.
    void editNote(int noteID, int userID, const std::string &noteData) {
        std::cout << "[TestCore] editNote called: noteID=" << noteID
                  << ", userID=" << userID << ", noteData=" << noteData << std::endl;
    }
}

// The ExtendedTask structure is defined within dispatcher.cpp (when DISPATCHER_TEST is defined)
// so we can instantiate it directly for testing.
using Dispatcher::ExtendedTask;

// Test case 1: Verify that a simple PING task is processed.
TEST(DispatcherTest, BasicPingTask) {
    using namespace Dispatcher;
    
    // Capture stdout so we can inspect the output.
    testing::internal::CaptureStdout();
    
    // Create a dispatcher with a single worker thread.
    create_threads(1);
    
    // Create and add a simple PING task.
    ExtendedTask pingTask(PING, 0, 0, "");
    addTaskForTesting(pingTask);
    
    // Allow some time for the task to be processed.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop the dispatcher.
    stopDispatcher();
    
    std::string output = testing::internal::GetCapturedStdout();
    // Verify that the output contains the expected PING task processing message.
    EXPECT_NE(output.find("Processing PING task. Responding with PONG"), std::string::npos);
}

// Test case 2: Verify that tasks are processed in static priority order.
// Here we enqueue an EDIT_NOTE task (static priority 2) and a PING task (static priority 5)
// and expect that the EDIT_NOTE task is processed first.
TEST(DispatcherTest, PriorityOrdering) {
    using namespace Dispatcher;
    
    testing::internal::CaptureStdout();
    
    // Create a dispatcher with a single thread to ensure ordered execution.
    create_threads(1);
    
    // Create a PING task and an EDIT_NOTE task.
    ExtendedTask pingTask(PING, 0, 0, "");
    ExtendedTask editTask(EDIT_NOTE, 101, 1, "Test edit content");
    
    // Enqueue the tasks. Order of insertion does not matter because the static priorities control order.
    addTaskForTesting(pingTask);
    addTaskForTesting(editTask);
    
    // Allow time for tasks to be processed.
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    stopDispatcher();
    
    std::string output = testing::internal::GetCapturedStdout();
    // Look for the worker messages that indicate task processing.
    size_t posEdit = output.find("Processing EDIT_NOTE task");
    size_t posPing = output.find("Processing PING task");
    
    EXPECT_NE(posEdit, std::string::npos);
    EXPECT_NE(posPing, std::string::npos);
    // Since EDIT_NOTE has a higher priority (lower static number), its message should appear before PING.
    EXPECT_LT(posEdit, posPing);
}

// Test case 3: Verify that a special CREATE_NOTE task is handled exclusively.
// The special task should stall other tasks until it completes.
TEST(DispatcherTest, SpecialTaskExecution) {
    using namespace Dispatcher;
    
    testing::internal::CaptureStdout();
    
    // Create a dispatcher with a single worker thread.
    create_threads(1);
    
    // Create a PING task to go before the special task.
    ExtendedTask pingTask(PING, 0, 0, "");
    addTaskForTesting(pingTask);
    
    // Now add a special CREATE_NOTE task.
    ExtendedTask createTask(CREATE_NOTE, 102, 2, "New note content");
    addTaskForTesting(createTask);
    
    // Add another PING task to follow the special task.
    ExtendedTask pingTask2(PING, 0, 0, "");
    addTaskForTesting(pingTask2);
    
    // Allow some time for all tasks to be processed.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    stopDispatcher();
    
    std::string output = testing::internal::GetCapturedStdout();
    // Verify that the special task messages appear.
    size_t posSpecialStart = output.find("[Dispatcher] Processing special CREATE_NOTE task exclusively.");
    size_t posTestCreate = output.find("[TestCore] createNote called:");
    size_t posSpecialEnd = output.find("[Dispatcher] Special CREATE_NOTE task completed.");
    
    EXPECT_NE(posSpecialStart, std::string::npos);
    EXPECT_NE(posTestCreate, std::string::npos);
    EXPECT_NE(posSpecialEnd, std::string::npos);
    
    // Verify ordering: the first PING task should occur before the special task,
    // and the second PING task should appear after the special task completes.
    size_t posPingFirst = output.find("Processing PING task");
    size_t posPingLast = output.rfind("Processing PING task");
    
    EXPECT_NE(posPingFirst, std::string::npos);
    EXPECT_NE(posPingLast, std::string::npos);
    EXPECT_LT(posPingFirst, posSpecialStart);
    EXPECT_GT(posPingLast, posSpecialEnd);
}
