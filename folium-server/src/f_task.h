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
    SING_IN,
    SING_UP,
    LOG_OUT,
    ERROR,
    SYSKILL
};

/**
 * @brief Struct for tasks in the server.
 */
struct F_Task {
    unsigned int thread_id;
    unsigned int progress;
    bool is_done;

    json data;
    
    F_TaskType type;
};

#endif // FOLSERV_TASK_H_