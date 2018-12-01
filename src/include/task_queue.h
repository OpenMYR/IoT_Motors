#pragma once

#include <os_type.h>

#define TASK_QUEUE_LENGTH 30

class task_queue
{
public:
    static os_event_t queue[TASK_QUEUE_LENGTH];

private:
    task_queue();
};

os_event_t task_queue::queue[TASK_QUEUE_LENGTH];