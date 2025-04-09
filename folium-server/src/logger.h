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

#include <string>
#include <sstream>

namespace logger
{
    /**
     * @brief Enumeration of logging levels.
     */
    enum LoggingLevel
    {
        STD,
        DEBUG,
        WARN,
        ERR
    };

    // Global logging configuration shared among all Logger instances.
    static LoggingLevel currentLevel;
    static bool loggingEnabled;

    void log(std::string s);

    void logErr(std::string s, bool do_log_trace = false);

    /**
     * @brief Logs a stream of items.
     */
    template <typename... Args>
    void logS(Args &&...args)
    {
        std::ostringstream oss;
        (oss << ... << args); // fold expression for streaming all args
        log(oss.str());
    }

    /**
     * @brief Returns a timestamp string in "HH:MM:SS" format.
     *
     * @return std::string The timestamp.
     */
    std::string static getTimestamp()
    {
        // get time
        time_t timestamp;
        time(&timestamp);

        // convert to string
        char timeString[std::size("Thh:mm:ssZ")];
        std::strftime(
            std::data(timeString),
            std::size(timeString),
            "%T",
            std::localtime(&timestamp));

        std::string s(timeString);

        return s;
    }

    /**
     * @brief Logger class for instance-based logging.
     *
     * Each instance can be created with its own identifier
     * (for example, "auth" or "db") which is included in the log messages.
     * Global settings for logging level and enabling/disabling logging are shared
     * among all logger instances.
     */
    class Logger
    {
    public:
        /**
         * @brief Constructs a new Logger instance.
         *
         * @param name A unique name that identifies the logger instance.
         */
        explicit Logger(const std::string &name)
            : instanceName(name) {}

        /**
         * @brief Set the logger's logging level.
         * Only messages with a severity equal or above the current level will be logged.
         *
         * @param level The desired logging level.
         */
        static void setLoggingLevel(LoggingLevel level);

        /**
         * @brief Enable or disable logging.
         *
         * @param enabled True to enable logging, false to disable.
         */
        static void setLoggingEnabled(bool enabled);

        /**
         * @brief Log a standard message.
         *
         * @param message The message to log.
         */
        void log(const std::string &message) const;

        /**
         * @brief Logs a stream of items.
         *
         * This is a variadic template function to simplify logging of various types.
         *
         * @tparam Args Types to log.
         * @param args Items to stream and log.
         */
        template <typename... Args>
        void logS(Args &&...args) const
        {
            std::ostringstream oss;
            (oss << ... << args); // Fold expression to stream all args.
            log(oss.str());
        }

        /**
         * @brief Log a debug message with the "DEBUG" tag.
         *
         * @param message The message to log.
         */
        void logDebug(const std::string &message) const;

        /**
         * @brief Log a warning message with the "WARN" tag.
         *
         * @param message The message to log.
         */
        void logWarn(const std::string &message) const;

        /**
         * @brief Log an error message with the "ERR" tag.
         *
         * Optionally logs a stack trace.
         *
         * @param message The message to log.
         * @param do_log_trace If true, include a stack trace in the log.
         */
        void logErr(const std::string &message, bool do_log_trace = false) const;

    private:
        // The unique instance name (e.g., "auth", "database", etc.)
        std::string instanceName;

        /**
         * @brief Builds the header string combining the instance name and timestamp.
         *
         * @return std::string The log header.
         */
        std::string getHeader() const;

        /**
         * @brief Internal function to print the log message to stdout if logging is enabled.
         * If the message does not end with a period or exclamation mark, one is appended.
         *
         * @param s The message to print.
         */
        void printLog(const std::string &s) const;
    };

} // namespace Logger

#endif // FOLSERV_LOGGER_H_
