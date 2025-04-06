#include <gtest/gtest.h>

#include "logger.h"

TEST(LoggerTest, LogMessage) {
    testing::internal::CaptureStdout();
    Logger::log("Hello, World!");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output, "Hello, World!\n");
}