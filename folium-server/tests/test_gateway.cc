#include <thread>
#include <chrono>

#include <gtest/gtest.h>
#include "httplib.h"

#include "http_gateway.h"

bool wait_until_port_open(const std::string& host, int port, int timeout_ms = 2000) {
    using namespace std::chrono_literals;

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

TEST(GatewayTest, PingRouteRespondsWithHelloWorld) {
    gateway::Gateway gw;
    gw.listen("127.0.0.1", 50105);

    ASSERT_TRUE(wait_until_port_open("127.0.0.1", 50105)) << "Server did not become ready in time";
    
    httplib::Client client("127.0.0.1", 50105);
    auto res = client.Get("/ping");

    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->body, "Hello World!\n");

    gw.stop();
}