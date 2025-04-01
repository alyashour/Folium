#ifndef FOL_TASK_H
#define FOL_TASK_H

enum Task_Type {
    PING,
    CREATE_NOTE,
    EDIT_NOTE,
    SING_IN,
    SING_UP,
    LOG_OUT
};

struct {
    unsigned int thread_id;
    unsigned int progress;
    bool is_done;
    Task_Type type;
} F_Task;

#endif // FOL_TASK_H