#include <gtest/gtest.h>
#include <string>
#include "logger.h"

// Test TC_LOG_01: LogMessage (Existing test)
TEST(LoggerTest, TC_LOG_01_LogMessage) {

    testing::internal::CaptureStdout();
    logger::log("Hello, World!");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.starts_with("[LOG | ")) << "Log should begin with header [LOG | ";
    EXPECT_TRUE(output.ends_with("] Hello, World!\n")) << "Log message should end with the expected text.";
}

// Test TC_LOG_02: LogErrorNoTrace
TEST(LoggerTest, TC_LOG_02_LogErrorNoTrace) {

    testing::internal::CaptureStdout();
    logger::logErr("Error occurred", false);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("[ERR |"), std::string::npos) << "Output should contain [ERR | header.";
    EXPECT_NE(output.find("Error occurred."), std::string::npos) << "Output should contain the error message.";
    EXPECT_EQ(output.find("Stack trace:"), std::string::npos) << "Output should not contain a stack trace.";
}

// Test TC_LOG_03: LogErrorWithTrace
TEST(LoggerTest, TC_LOG_03_LogErrorWithTrace) {

    testing::internal::CaptureStdout();
    logger::logErr("Critical failure", true);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("[ERR |"), std::string::npos) << "Output should include the error header.";
    EXPECT_NE(output.find("Critical failure."), std::string::npos) << "Output should include the error message.";
    EXPECT_NE(output.find("Stack trace:"), std::string::npos) << "Output should include a stack trace.";
}

// Test TC_LOG_04: InstanceLoggerLog
TEST(LoggerTest, TC_LOG_04_InstanceLoggerLog) {

    testing::internal::CaptureStdout();
    logger::Logger myLogger("TestInstance");
    myLogger.log("Instance log message");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("[LOG | TestInstance |"), std::string::npos)
        << "Log header should include 'TestInstance'.";
    EXPECT_NE(output.find("Instance log message."), std::string::npos)
        << "Log message should include the instance message with punctuation.";
}

// Test TC_LOG_05: InstanceLoggerLogS
TEST(LoggerTest, TC_LOG_05_InstanceLoggerLogS) {

    testing::internal::CaptureStdout();
    logger::Logger myLogger("TestInstance");
    myLogger.logS("Multiple ", "argument ", "log");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Multiple argument log."), std::string::npos)
        << "The logS output should properly concatenate all arguments.";
}

// Test TC_LOG_06: LoggingLevelControl
TEST(LoggerTest, TC_LOG_06_LoggingLevelControl) {

    logger::setLoggingEnabled(false);
    testing::internal::CaptureStdout();
    logger::log("This message should not appear");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.empty()) << "Logging disabled: no output expected.";
    logger::setLoggingEnabled(true);

    // Test logging level filtering.
    logger::setLoggingLevel(logger::ERR);  // Only errors should be printed
    testing::internal::CaptureStdout();
    logger::logDebug("Debug message should be suppressed");
    output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.empty()) << "Debug message should be suppressed at ERR level.";
    // Reset logging level to standard.
    logger::setLoggingLevel(logger::STD);
}

// Test TC_LOG_07: GetTimestampFormat
TEST(LoggerTest, TC_LOG_07_GetTimestampFormat) {

    std::string timestamp = logger::getTimestamp();
    EXPECT_EQ(timestamp.size(), 8) << "Timestamp should be in HH:MM:SS format (8 characters long).";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
