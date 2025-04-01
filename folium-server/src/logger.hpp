/**
 * This is a custom logger.
 * Logs to stdout with timestamps, color, and trace.
 * 
 * HAS state.
 */

#ifndef FOL_LOGGER
#define FOL_LOGGER

#include "string"

namespace Logger {

    enum LoggingLevel {
        STD,
        DEBUG,
        WARN,
        ERR
    };

    /* State and config */
    
    /// @brief Set the loggers logging level
    /// @param level 
    void set_logging_level(const LoggingLevel& level);

    /// @brief Disable or enable logging
    /// @param v 
    void set_logging_enabled(bool v);

    /* Logging */

    /// @brief Logs to stdout
    /// @param s The string to log
    void log(std::string s);

    /// @brief Logs to stdout with "DEBUG" tag
    /// @param s The string to log
    void log_debug(std::string s);

    /// @brief Logs to stdout with "WARN" tag
    /// @param s The string to log
    void log_warn(std::string s);

    /// @brief Logs to stdout with "ERR" tag
    /// @param s The string to log
    /// @param do_log_trace Should the logger print the trace?
    void log_err(std::string s, bool do_log_trace=false);
}

#endif FOL_LOGGER