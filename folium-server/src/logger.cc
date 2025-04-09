#include <string>
#include <iostream>
#include <ctime>

#include "logger.h"

std::string static getHeader(const std::string &s)
{
    return "[" + s + " | " + logger::getTimestamp() + "] ";
}

void logger::log(std::string s)
{
    // add a period if it doesn't end with one
    if (!s.ends_with('.') && !s.ends_with('!'))
    {
        s += ".";
    }

    std::cout << getHeader("LOG") << s << std::endl;
}

void logger::logErr(std::string s, bool do_log_trace)
{
    // add a period if it doesn't end with one
    if (!s.ends_with('.') && !s.ends_with('!'))
    {
        s += ".";
    }

    std::cout << getHeader("ERR") << s << std::endl;
}

namespace logger
{
    // Returns a header string combining the instance name and timestamp.
    std::string Logger::getHeader() const
    {
        return "[LOG | " + instanceName + " | " + getTimestamp() + "] ";
    }

    // Internal function: prints a message if logging is enabled.
    // Appends a period if the message doesn't end with one of the expected punctuation.
    void Logger::printLog(const std::string &s) const
    {
        if (!loggingEnabled)
            return;
        std::string msg = s;
        if (!msg.empty() && msg.back() != '.' && msg.back() != '!')
        {
            msg += ".";
        }
        std::cout << getHeader() << msg << std::endl;
    }

    // Log a standard message (severity STD).
    void Logger::log(const std::string &s) const
    {
        if (static_cast<int>(STD) >= static_cast<int>(currentLevel))
        {
            printLog(s);
        }
    }

    // Log a debug message (severity DEBUG).
    void Logger::logDebug(const std::string &s) const
    {
        if (static_cast<int>(DEBUG) >= static_cast<int>(currentLevel))
        {
            printLog("DEBUG: " + s);
        }
    }

    // Log a warning message (severity WARN).
    void Logger::logWarn(const std::string &s) const
    {
        if (static_cast<int>(WARN) >= static_cast<int>(currentLevel))
        {
            printLog("WARN: " + s);
        }
    }

    // Log an error message (severity ERR). Optionally, print a stack trace.
    void Logger::logErr(const std::string &s, bool do_log_trace) const
    {
        if (static_cast<int>(ERR) >= static_cast<int>(currentLevel))
        {
            printLog("ERR: " + s);
            if (do_log_trace)
            {
                // For a real implementation, you could integrate a platform-specific stack tracing library.
                printLog("Stack trace: (not implemented)");
            }
        }
    }
} // namespace Logger
