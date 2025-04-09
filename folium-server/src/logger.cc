#include <string>
#include <iostream>
#include <ctime>
#include <sstream>
#include "logger.h"  // Make sure this header declares the LoggingLevel enum.

namespace Logger {

class Logger {
private:
    std::string instanceName;  // For example: "auth", "gateway", etc.

    // Global logging settings shared by all logger instances.
    static LoggingLevel currentLevel;
    static bool loggingEnabled;

    // Helper function: returns a timestamp string in "HH:MM:SS" format.
    static std::string getTimestamp() {
        time_t now;
        time(&now);
        char timeString[9];  // "HH:MM:SS" plus null terminator.
        std::strftime(timeString, sizeof(timeString), "%T", std::localtime(&now));
        return std::string(timeString);
    }

    // Returns a header string combining the instance name and timestamp.
    std::string getHeader() const {
        return "[LOG | " + instanceName + " | " + getTimestamp() + "] ";
    }

    // Internal function: prints a message if logging is enabled.
    // Appends a period if the message doesn't end with one of the expected punctuation.
    void printLog(const std::string &s) const {
        if (!loggingEnabled) return;
        std::string msg = s;
        if (!msg.empty() && msg.back() != '.' && msg.back() != '!') {
            msg += ".";
        }
        std::cout << getHeader() << msg << std::endl;
    }

public:
    // Constructor accepting the instance name.
    explicit Logger(const std::string &name)
        : instanceName(name) {}

    // Static setters for controlling the logging level and whether logging is enabled.
    static void setLoggingLevel(LoggingLevel level) {
        currentLevel = level;
    }
    
    static void setLoggingEnabled(bool v) {
        loggingEnabled = v;
    }

    // Log a standard message (severity STD).
    void log(const std::string &s) const {
        if (static_cast<int>(STD) >= static_cast<int>(currentLevel)) {
            printLog(s);
        }
    }

    // Log a debug message (severity DEBUG).
    void logDebug(const std::string &s) const {
        if (static_cast<int>(DEBUG) >= static_cast<int>(currentLevel)) {
            printLog("DEBUG: " + s);
        }
    }

    // Log a warning message (severity WARN).
    void logWarn(const std::string &s) const {
        if (static_cast<int>(WARN) >= static_cast<int>(currentLevel)) {
            printLog("WARN: " + s);
        }
    }

    // Log an error message (severity ERR). Optionally, print a stack trace.
    void logErr(const std::string &s, bool do_log_trace = false) const {
        if (static_cast<int>(ERR) >= static_cast<int>(currentLevel)) {
            printLog("ERR: " + s);
            if (do_log_trace) {
                // For a real implementation, you could integrate a platform-specific stack tracing library.
                printLog("Stack trace: (not implemented)");
            }
        }
    }
};

// Initialize static members.
LoggingLevel Logger::currentLevel = STD;
bool Logger::loggingEnabled = true;

} // namespace Logger
