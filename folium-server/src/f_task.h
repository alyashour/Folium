/**
 * @file f_task.h
 * @brief Task structure and types for the Folium server.
 */

#ifndef FOLSERV_TASK_H_
#define FOLSERV_TASK_H_

/**
 * @brief Task types handled by the server.
 */
enum Task_Type {
    PING,
    CREATE_NOTE,
    EDIT_NOTE,
    SING_IN,
    SING_UP,
    LOG_OUT
};

/**
 * @brief Struct for tasks in the server.
 */
struct F_Task {
    unsigned int thread_id;
    unsigned int progress;
    bool is_done;
    Task_Type type;
};

#endif // FOLSERV_TASK_H_