#include <string>

#include <gtest/gtest.h>

#include "logger.h"

TEST(LoggerTest, LogMessage) {
    testing::internal::CaptureStdout();
    logger::log("Hello, World!");
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_TRUE(output.starts_with("[LOG | ")) << "Log didn't begin with log.";
    EXPECT_TRUE(output.ends_with("] Hello, World!\n")) << "Log didn't end with correct message.";
}