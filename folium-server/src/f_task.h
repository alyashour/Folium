/**
 * @file f_task.h
 * @brief Task structure and types for the Folium server.
 */

#ifndef FOLSERV_TASK_H_
#define FOLSERV_TASK_H_

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief Task types handled by the server.
 *
 * Make sure you have an entry for each route in routes.md:
 */
enum F_TaskType
{
    // System / Utility
    PING,
    SYSKILL,
    ERROR,

    // Auth
    REGISTER,             // POST /api/auth/register
    SIGN_IN,              // POST /api/auth/login
    LOG_OUT,              // POST /api/auth/logout
    AUTH_REFRESH,         // POST /api/auth/refresh-token
    AUTH_CHANGE_PASSWORD, // POST /api/auth/change-password

    // Classes
    GET_CLASSES,           // GET /api/classes
    GET_ME_CLASSES,        // GET /api/me/classes
    POST_ME_CLASSES,       // POST /api/me/classes
    PUT_CLASS,             // PUT /api/me/classes/{classId}
    DELETE_CLASS,          // DELETE /api/me/classes/{classId}
    GET_CLASS_DETAILS,     // GET /api/me/classes/{classId}
    GET_CLASS_OWNER,       // GET /api/me/classes/{classId}/owner
    GET_CLASS_NAME,        // GET /api/me/classes/{classId}/name
    GET_CLASS_DESCRIPTION, // GET /api/me/classes/{classId}/description
    GET_CLASS_BIGNOTE,     // GET /api/me/classes/{classId}/bigNote
    GET_CLASS_TITLE,       // GET /api/me/classes/{classId}/title

    // Notes
    POST_UPLOAD_NOTE,    // POST /api/me/classes/{classId}/upload-note
    PUT_BIGNOTE_EDIT,    // PUT /api/me/classes/{classId}/bigNote/edit-note
    GET_BIGNOTE_HISTORY, // GET /api/me/classes/{classId}/bigNote/history
    GET_BIGNOTE_EXPORT,  // GET /api/me/classes/{classId}/bigNote/export

    // Optionally keep these if your code references them
    CREATE_NOTE,
    EDIT_NOTE
};

/**
 * @brief Struct for tasks in the server.
 */
struct F_Task
{
    unsigned int threadId_ = 0;
    unsigned int progress_ = 0;
    bool isDone_ = false;

    json data_;
    F_TaskType type_;

    /**
     * @brief Creates an empty task.
     * @deprecated Use F_Task(F_TaskType type) instead.
     */
    F_Task() {}

    F_Task(F_TaskType type)
        : type_(type)
    {
    }

    int getPriority() const
    {
        // Lower return value => higher priority
        switch (type_)
        {
        // System / Utility
        case SYSKILL:
            // Highest priority, should be processed immediately
            return 1;
        case PING:
            // Quick “ping/pong” check
            return 2;

        // Auth
        case SIGN_IN:
            // Logging in is often time-sensitive
            return 3;
        case REGISTER:
        case AUTH_REFRESH:
            // Still relatively high priority
            return 4;
        case AUTH_CHANGE_PASSWORD:
        case LOG_OUT:
            // Slightly lower than sign-in/registration
            return 5;

        // Classes
        // GET/POST calls are generally mid-level priority
        case GET_CLASSES:
        case GET_ME_CLASSES:
        case POST_ME_CLASSES:
        case GET_CLASS_DETAILS:
        case GET_CLASS_OWNER:
        case GET_CLASS_NAME:
        case GET_CLASS_DESCRIPTION:
        case GET_CLASS_BIGNOTE:
        case GET_CLASS_TITLE:
            return 6;

        // Updating or deleting classes is a bit more “expensive”
        case PUT_CLASS:
        case DELETE_CLASS:
            return 7;

        // Notes
        case POST_UPLOAD_NOTE:
        case CREATE_NOTE:
            // Creating/uploading typically mid-range
            return 7;
        case PUT_BIGNOTE_EDIT:
        case EDIT_NOTE:
            // Editing existing notes
            return 8;
        case GET_BIGNOTE_HISTORY:
        case GET_BIGNOTE_EXPORT:
            // Reading/exporting a big note
            return 8;

        // Default or error
        case ERROR:
        default:
            // Lowest priority
            return 10;
        }
    }
};

#endif // FOLSERV_TASK_H_
