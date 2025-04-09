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
 */
enum F_TaskType {
    PING,
    CREATE_NOTE,
    REGISTER,
    EDIT_NOTE,
    SIGN_IN,
    SIGN_UP,
    LOG_OUT,
    ERROR,
    SYSKILL
};

/**
 * @brief Struct for tasks in the server.
 */
struct F_Task {
    
    unsigned int threadId_;
    unsigned int progress_;
    bool isDone_;

    json data_;
    
    F_TaskType type_;

    /**
     * @brief Creates an empty task.
     * @deprecated Use F_Task(F_TaskType type) instead.
     */
    F_Task() {}

    F_Task(F_TaskType type)
        : type_(type)
    {}

    // TODO: IMPROVE THIS SYSTEM
    int getPriority() const {
        if (type_ == F_TaskType::PING || type_ == F_TaskType::SYSKILL) {
            return 1;
        } else if (type_ == F_TaskType::SIGN_IN) {
            return 2;
        } else {
            return 10;
        }
    }
};

#endif // FOLSERV_TASK_H_