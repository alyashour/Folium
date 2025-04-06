/**
 * @file logger.h
 * @brief Custom logger for the Folium server.
 *
 * This file defines a custom logging utility for the Folium server. 
 * Really just for development.
 * 
 * Also timestamps, color coding, and stack traces (maybe).
 *
 * @section Responsibilities
 * - Log messages with different levels (STD, DEBUG, WARN, ERR).
 * - Provide configuration options for logging levels and enabling/disabling logging.
 * - Enhance debugging and error tracking with optional stack traces.
 */

#ifndef FOLSERV_LOGGER_H_
#define FOLSERV_LOGGER_H_

#include "string"

namespace Logger {

    /**
     * @brief Enumeration of logging levels.
     */
    enum LoggingLevel {
        STD,
        DEBUG,
        WARN,
        ERR
    };

    /* State and config */

    /**
     * @brief Set the logger's logging level.
     * @param level The desired logging level.
     */
    void set_logging_level(const LoggingLevel& level);

    /**
     * @brief Enable or disable logging.
     * @param v True to enable logging, false to disable.
     */
    void set_logging_enabled(bool v);

    /* Logging */

    /**
     * @brief Log a standard message to stdout.
     * @param s The string to log.
     */
    void log(std::string s);

    /**
     * @brief Log a debug message to stdout with the "DEBUG" tag.
     * @param s The string to log.
     */
    void log_debug(std::string s);

    /**
     * @brief Log a warning message to stdout with the "WARN" tag.
     * @param s The string to log.
     */
    void log_warn(std::string s);

    /**
     * @brief Log an error message to stdout with the "ERR" tag.
     * @param s The string to log.
     * @param do_log_trace If true, include a stack trace in the log.
     */
    void log_err(std::string s, bool do_log_trace = false);
}

#endif // FOLSERV_LOGGER_H_