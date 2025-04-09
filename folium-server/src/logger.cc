#include <string>
#include <iostream>
#include <ctime>
#include <sstream>
#include "logger.h"

namespace {

// Helper function: returns a timestamp in "HH:MM:SS" format.
static std::string getTimestamp() {
    time_t timestamp;
    time(&timestamp);
    char timeString[9];  // "HH:MM:SS" is 8 characters plus null terminator.
    std::strftime(timeString, sizeof(timeString), "%T", std::localtime(&timestamp));
    return std::string(timeString);
}

// Helper function: returns a header string with the timestamp.
static std::string getHeader() {
    return "[LOG | " + getTimestamp() + "] ";
}

} // end anonymous namespace

namespace Logger {

// Global state for controlling logging behavior.
static LoggingLevel currentLevel = STD;  // Default level (most verbose).
static bool loggingEnabled = true;

// Set the logger's logging level (only messages with severity >= currentLevel will be logged).
void setLoggingLevel(const LoggingLevel &level) {
    currentLevel = level;
}

// Enable or disable logging globally.
void setLoggingEnabled(bool v) {
    loggingEnabled = v;
}

// Internal function: prints a message if logging is enabled.
// If the message does not end with '.' or '!', adds a period.
static void printLog(const std::string &s) {
    if (!loggingEnabled) return;
    std::string msg = s;
    if (!msg.empty() && msg.back() != '.' && msg.back() != '!') {
        msg += ".";
    }
    std::cout << getHeader() << msg << std::endl;
}

// Log a standard message with severity STD (0).
void log(std::string s) {
    // STD messages have severity 0.
    if (static_cast<int>(STD) >= static_cast<int>(currentLevel)) {
        printLog(s);
    }
}

// Log a debug message with severity DEBUG (1).
void logDebug(std::string s) {
    if (static_cast<int>(DEBUG) >= static_cast<int>(currentLevel)) {
        printLog("DEBUG: " + s);
    }
}

// Log a warning message with severity WARN (2).
void logWarn(std::string s) {
    if (static_cast<int>(WARN) >= static_cast<int>(currentLevel)) {
        printLog("WARN: " + s);
    }
}

// Log an error message with severity ERR (3). Optionally prints a stack trace.
void logErr(std::string s, bool do_log_trace) {
    if (static_cast<int>(ERR) >= static_cast<int>(currentLevel)) {
        printLog("ERR: " + s);
        if (do_log_trace) {
            // A placeholder for a stack trace. For a real implementation, you might integrate
            // a platform-specific stack tracing library.
            printLog("Stack trace: (not implemented)");
        }
    }
}

} // namespace Logger
