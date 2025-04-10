
#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include "httplib.h"
#include "http_gateway.h"
#include "fifo_channel.h"
#include "f_task.h"

using namespace std::chrono_literals;
using json = nlohmann::json;

// Minimal mock FIFO channel to simulate IPC in memory.
class MockFifoChannel : public ipc::FifoChannel {
public:
    MockFifoChannel() : ipc::FifoChannel("", O_RDWR, false) {}

    bool read(F_Task &task) override {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!inQueue.empty()) {
            task = inQueue.front();
            inQueue.erase(inQueue.begin());
            return true;
        }
        return false;
    }

    bool send(const F_Task &task) override {
        std::lock_guard<std::mutex> lock(mutex_);
        outQueue.push_back(task);
        return true;
    }

    // Helper: Push a task into the input queue.
    void pushTask(const F_Task &task) {
        std::lock_guard<std::mutex> lock(mutex_);
        inQueue.push_back(task);
    }

    // Helper: Retrieve the tasks that have been sent.
    std::vector<F_Task> getSentTasks() {
        std::lock_guard<std::mutex> lock(mutex_);
        return outQueue;
    }

private:
    std::vector<F_Task> inQueue;
    std::vector<F_Task> outQueue;
    std::mutex mutex_;
};

// Helper function to wait until a port is open.
bool wait_until_port_open(const std::string& host, int port, int timeout_ms = 2000) {
    int elapsed = 0;
    while (elapsed < timeout_ms) {
        httplib::Client client(host, port);
        auto res = client.Get("/ping");
        if (res) return true;
        std::this_thread::sleep_for(50ms);
        elapsed += 50;
    }
    return false;
}

// ----------------- Test Cases ----------------- //

// TC_GATEWAY_01 – PingRouteRespondsWithPong
TEST(GatewayTest, TC_GATEWAY_01_PingRouteRespondsWithPong) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    gateway::Gateway gw(inChannel, outChannel);
    // Start listening on port 50105.
    gw.listen("127.0.0.1", 50105);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50105));
    httplib::Client client("127.0.0.1", 50105);
    auto res = client.Get("/ping");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "Pong!\n");
    gw.stop();
}

// TC_GATEWAY_02 – PingCoreRouteReturnsPong
TEST(GatewayTest, TC_GATEWAY_02_PingCoreRouteReturnsPong) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    
    // Preload a dummy response for PING.
    F_Task dummyPing(F_TaskType::PING);
    dummyPing.data_ = { {"message", "pong!"} };
    inChannel.pushTask(dummyPing);

    gateway::Gateway gw(inChannel, outChannel);
    gw.listen("127.0.0.1", 50106);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50106));
    httplib::Client client("127.0.0.1", 50106);
    auto res = client.Get("/ping-core");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    json jsonRes = json::parse(res->body);
    EXPECT_TRUE(jsonRes.contains("message"));
    EXPECT_EQ(jsonRes["message"], "pong!");
    gw.stop();
}

// TC_GATEWAY_03 – RegisterRouteValid
TEST(GatewayTest, TC_GATEWAY_03_RegisterRouteValid) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    
    // Preload dummy register response.
    F_Task regResponse(F_TaskType::REGISTER);
    regResponse.data_ = { {"message", "User registered"}, {"userId", 42} };
    inChannel.pushTask(regResponse);

    gateway::Gateway gw(inChannel, outChannel);
    gw.listen("127.0.0.1", 50107);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50107));

    httplib::Client client("127.0.0.1", 50107);
    json requestBody = { {"username", "testuser"}, {"password", "testpass"} };
    auto res = client.Post("/api/auth/register", requestBody.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    json responseJson = json::parse(res->body);
    EXPECT_TRUE(responseJson.contains("message"));
    EXPECT_TRUE(responseJson.contains("userId"));

    gw.stop();
}

// TC_GATEWAY_04 – RegisterRouteInvalidJSON
TEST(GatewayTest, TC_GATEWAY_04_RegisterRouteInvalidJSON) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    gateway::Gateway gw(inChannel, outChannel);
    gw.listen("127.0.0.1", 50108);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50108));

    httplib::Client client("127.0.0.1", 50108);
    std::string malformedJson = "This is not JSON!";
    auto res = client.Post("/api/auth/register", malformedJson, "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 400);
    json responseJson = json::parse(res->body);
    EXPECT_TRUE(responseJson.contains("error"));
    gw.stop();
}

// TC_GATEWAY_05 – LoginRouteValid
TEST(GatewayTest, TC_GATEWAY_05_LoginRouteValid) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    
    F_Task loginResponse(F_TaskType::REGISTER); // (Assuming REGISTER is used here as per your Gateway code)
    loginResponse.data_ = { {"token", "token_for_testuser"} };
    inChannel.pushTask(loginResponse);

    gateway::Gateway gw(inChannel, outChannel);
    gw.listen("127.0.0.1", 50109);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50109));

    httplib::Client client("127.0.0.1", 50109);
    json requestBody = { {"username", "testuser"}, {"password", "testpass"} };
    auto res = client.Post("/api/auth/login", requestBody.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    json responseJson = json::parse(res->body);
    EXPECT_TRUE(responseJson.contains("token"));
    EXPECT_EQ(responseJson["token"], "token_for_testuser");
    gw.stop();
}

// TC_GATEWAY_06 – LoginRouteInvalid
TEST(GatewayTest, TC_GATEWAY_06_LoginRouteInvalid) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    
    F_Task errorResponse(F_TaskType::ERROR);
    errorResponse.data_ = { {"error", "Invalid credentials"} };
    inChannel.pushTask(errorResponse);

    gateway::Gateway gw(inChannel, outChannel);
    gw.listen("127.0.0.1", 50110);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50110));

    httplib::Client client("127.0.0.1", 50110);
    json requestBody = { {"username", "wronguser"}, {"password", "wrongpass"} };
    auto res = client.Post("/api/auth/login", requestBody.dump(), "application/json");
    ASSERT_NE(res, nullptr);
    // Accept 400 or 401 as an invalid login status.
    EXPECT_TRUE(res->status == 400 || res->status == 401);
    json responseJson = json::parse(res->body);
    EXPECT_TRUE(responseJson.contains("error"));
    gw.stop();
}

// TC_GATEWAY_07 – LogoutRoute
TEST(GatewayTest, TC_GATEWAY_07_LogoutRoute) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    gateway::Gateway gw(inChannel, outChannel);
    gw.listen("127.0.0.1", 50111);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50111));

    httplib::Client client("127.0.0.1", 50111);
    auto res = client.Post("/api/auth/logout", "", "application/json");
    ASSERT_NE(res, nullptr);
    // Since logout route does not set content, we only check status.
    EXPECT_EQ(res->status, 200);
    gw.stop();
}

// TC_GATEWAY_08 – SignalShutdown
TEST(GatewayTest, TC_GATEWAY_08_SignalShutdown) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    gateway::Gateway gw(inChannel, outChannel);
    gw.signal_shutdown();
    auto responses = outChannel.getSentTasks();
    bool foundSyskill = false;
    for (const auto &task : responses) {
        if (task.type_ == F_TaskType::SYSKILL) {
            foundSyskill = true;
            break;
        }
    }
    EXPECT_TRUE(foundSyskill) << "Expected a SYSKILL task in the output FIFO.";
}

// TC_GATEWAY_09 – ListenStopCycle
TEST(GatewayTest, TC_GATEWAY_09_ListenStopCycle) {

    MockFifoChannel inChannel;
    MockFifoChannel outChannel;
    gateway::Gateway gw(inChannel, outChannel);
    gw.listen("127.0.0.1", 50112);
    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50112));

    httplib::Client client("127.0.0.1", 50112);
    auto res = client.Get("/ping");
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);

    gw.stop();
    // Optionally, we could try sending another request here and expect connection failure.
}

// TC_GATEWAY_10 – ExtractJWT_Valid
TEST(GatewayTest, TC_GATEWAY_10_ExtractJWT_Valid) {

    httplib::Request req;
    req.headers["Authorization"] = "Bearer sometoken";
    std::string token = extractJWT(req);
    EXPECT_EQ(token, "sometoken");
}

// TC_GATEWAY_11 – ExtractJWT_Invalid
TEST(GatewayTest, TC_GATEWAY_11_ExtractJWT_Invalid) {

    httplib::Request req;
    // Option 1: No header present - expect exception.
    EXPECT_THROW({
        extractJWT(req);
    }, std::exception);

    // Option 2: Malformed header.
    req.headers["Authorization"] = "Basic abc123";
    EXPECT_THROW({
        extractJWT(req);
    }, std::exception);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
