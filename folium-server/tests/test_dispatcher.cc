#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <map>
#include <stdexcept>
#include <filesystem>

// Include dispatcher and supporting headers.
#include "dispatcher.h"
#include "f_task.h"
#include "fifo_channel.h"

using namespace dispatcher;
using namespace std::chrono_literals;

// -------------------
// Mock FIFO Channel for Testing
// -------------------
class MockFifoChannel : public ipc::FifoChannel {
public:
    // Constructor: call base with dummy parameters.
    MockFifoChannel() : ipc::FifoChannel("", O_RDWR, false) {}

    // Override read() to pop from an in-memory queue.
    bool read(F_Task &task) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!inQueue.empty()) {
            task = inQueue.front();
            inQueue.erase(inQueue.begin());
            return true;
        }
        return false;
    }

    // Override send() to capture output.
    bool send(const F_Task &task) override {
        std::lock_guard<std::mutex> lock(mutex_);
        outQueue.push_back(task);
        return true;
    }

    // Helper: push a task into the input queue.
    void pushTask(const F_Task &task) {
        std::lock_guard<std::mutex> lock(mutex_);
        inQueue.push_back(task);
    }

    // Helper: retrieve all tasks that have been sent.
    std::vector<F_Task> getSentTasks() {
        std::lock_guard<std::mutex> lock(mutex_);
        return outQueue;
    }
private:
    std::vector<F_Task> inQueue;
    std::vector<F_Task> outQueue;
    std::mutex mutex_;
};

// -------------------
// For testing, override Core functions as needed.
// (Here, for CREATE_NOTE tasks, we count calls.)
namespace Core {
    std::atomic<int> createNoteCallCount{0};
    void createNote(int noteID, int userID, const std::string &noteData) {
        createNoteCallCount++;
    }
}

// -------------------
// Test Cases for Dispatcher
// -------------------

// TC_DSP_01 – InitializationTest
TEST(DispatcherTest, TC_DSP_01_InitializationTest) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    F_Task handshake(F_TaskType::PING);
    handshake.data_ = { {"message", "handshake"} };
    inChannel.pushTask(handshake);

    Dispatcher dispatcherInstance(inChannel, outChannel, 2);
    auto responses = outChannel.getSentTasks();
    ASSERT_FALSE(responses.empty()) << "Expected handshake response.";
    EXPECT_EQ(responses.front().type_, F_TaskType::PING)
        << "Handshake response should be of type PING.";
}

// TC_DSP_02 – CreateThreadsTest
TEST(DispatcherTest, TC_DSP_02_CreateThreadsTest) {
    
    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    inChannel.pushTask(F_Task(F_TaskType::PING)); // Handshake
    Dispatcher dispatcherInstance(inChannel, outChannel, 3);
    dispatcherInstance.addTask(ExtendedTask(F_TaskType::PING, 1, 0, "Test Ping"));
    std::this_thread::sleep_for(300ms);
    auto responses = outChannel.getSentTasks();
    EXPECT_FALSE(responses.empty()) << "Expected task processing by worker threads.";
}

// TC_DSP_03 – PingTaskProcessingTest
TEST(DispatcherTest, TC_DSP_03_PingTaskProcessingTest) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    F_Task handshake(F_TaskType::PING);
    handshake.data_ = { {"message", "handshake"} };
    inChannel.pushTask(handshake);
    Dispatcher dispatcherInstance(inChannel, outChannel, 2);
    outChannel.getSentTasks(); // Clear handshake responses.
    F_Task pingTask(F_TaskType::PING);
    pingTask.data_ = { {"message", "process me"} };
    inChannel.pushTask(pingTask);
    std::thread dispThread(&Dispatcher::start, &dispatcherInstance);
    std::this_thread::sleep_for(300ms);
    F_Task killTask(F_TaskType::SYSKILL);
    inChannel.pushTask(killTask);
    dispThread.join();

    auto responses = outChannel.getSentTasks();
    bool foundPong = false;
    for (const auto& resp : responses) {
        if (resp.type_ == F_TaskType::PING &&
            resp.data_.contains("message") &&
            resp.data_["message"] == "pong from dispatch") {
            foundPong = true;
            break;
        }
    }
    EXPECT_TRUE(foundPong) << "Expected a PING response with 'pong from dispatch'.";
}

// TC_DSP_04 – SpecialCreateNoteTaskTest
TEST(DispatcherTest, TC_DSP_04_SpecialCreateNoteTaskTest) {

    Core::createNoteCallCount = 0;
    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    inChannel.pushTask(F_Task(F_TaskType::PING)); // Handshake
    Dispatcher dispatcherInstance(inChannel, outChannel, 2);
    dispatcherInstance.addTask(ExtendedTask(F_TaskType::CREATE_NOTE, 100, 1, "New note content"));
    std::this_thread::sleep_for(300ms);
    EXPECT_EQ(Core::createNoteCallCount.load(), 1)
        << "CREATE_NOTE should trigger Core::createNote exactly once.";
}

// TC_DSP_05 – SysKillShutdownTest
TEST(DispatcherTest, TC_DSP_05_SysKillShutdownTest) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    inChannel.pushTask(F_Task(F_TaskType::PING)); // Handshake
    Dispatcher dispatcherInstance(inChannel, outChannel, 2);
    std::thread dispThread(&Dispatcher::start, &dispatcherInstance);
    std::this_thread::sleep_for(100ms);
    F_Task sysKill(F_TaskType::SYSKILL);
    inChannel.pushTask(sysKill);
    F_Task extraPing(F_TaskType::PING);
    extraPing.data_ = { {"message", "should not process"} };
    inChannel.pushTask(extraPing);
    dispThread.join();
    auto responses = outChannel.getSentTasks();
    bool processedAfterKill = false;
    for (const auto& resp : responses) {
        if (resp.type_ == F_TaskType::PING &&
            resp.data_.contains("message") &&
            resp.data_["message"] == "pong from dispatch") {
            processedAfterKill = true;
            break;
        }
    }
    EXPECT_FALSE(processedAfterKill)
        << "No tasks should be processed after SYSKILL.";
}

// TC_DSP_06 – CallbackInvocationTest
TEST(DispatcherTest, TC_DSP_06_CallbackInvocationTest) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    std::atomic<int> callbackCount{0};
    auto callback = [&callbackCount](const ipc::Task &task) {
        callbackCount++;
    };

    inChannel.pushTask(F_Task(F_TaskType::PING)); // Handshake
    Dispatcher dispatcherInstance(inChannel, outChannel, 2);
    std::thread listenerThread([&dispatcherInstance, &callback]() {
        dispatcherInstance.startListener(callback);
    });

    // Inject three additional tasks.
    for (int i = 0; i < 3; i++) {
        F_Task t(F_TaskType::PING);
        t.data_ = { {"message", "callback test"} };
        inChannel.pushTask(t);
    }
    std::this_thread::sleep_for(300ms);
    F_Task killTask(F_TaskType::SYSKILL);
    inChannel.pushTask(killTask);
    listenerThread.join();
    EXPECT_EQ(callbackCount.load(), 4)
        << "Expected 4 callback invocations (handshake + 3 tasks).";
}

// TC_DSP_07 – PerformanceLoadTest
TEST(DispatcherTest, TC_DSP_07_PerformanceLoadTest) {

    const int numTasks = 1000;
    MockFifoChannel inChannel;
    MockFifoChannel outChannel;

    // Handshake.
    F_Task handshake(F_TaskType::PING);
    handshake.data_ = { {"message", "handshake"} };
    inChannel.pushTask(handshake);

    // Create dispatcher with 8 worker threads.
    Dispatcher dispatcherInstance(inChannel, outChannel, 8);

    // Enqueue a large number of PING tasks.
    for (int i = 0; i < numTasks; i++) {
        F_Task task(F_TaskType::PING);
        task.data_ = { {"message", "load test " + std::to_string(i)} };
        inChannel.pushTask(task);
    }

    auto startTime = std::chrono::steady_clock::now();
    // Run the dispatcher in a background thread.
    std::thread dispThread(&Dispatcher::start, &dispatcherInstance);

    // Wait until we have processed at least numTasks+1 responses (handshake + tasks)
    while (true) {
        auto responses = outChannel.getSentTasks();
        if (responses.size() >= static_cast<size_t>(numTasks + 1))
            break;
        std::this_thread::sleep_for(50ms);
    }
    // Send SYSKILL to stop dispatcher processing.
    F_Task killTask(F_TaskType::SYSKILL);
    inChannel.pushTask(killTask);
    dispThread.join();
    auto endTime = std::chrono::steady_clock::now();
    auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // Check that all tasks were processed.
    auto responses = outChannel.getSentTasks();
    EXPECT_GE(responses.size(), static_cast<size_t>(numTasks + 1))
        << "Expected at least " << numTasks + 1 << " responses.";
    // Assert that processing completes under 5 seconds.
    EXPECT_LT(durationMs, 5000)
        << "Performance test exceeded acceptable duration (5 seconds).";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
