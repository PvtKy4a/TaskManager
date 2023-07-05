#include "TaskManager.h"
#include <stdlib.h>
#include "Queue.h"

typedef struct {
  void (* task_function)(void *);
  void * context;
  tm_task_run_mode_t run_mode;
  uint32_t delay;
  uint32_t timeout;
} Task_t;

typedef struct {
  Queue_t * task_queue;
  Task_t ** task_list;
  uint8_t capacity;
  uint8_t size;
  void (* disable_interrupt)(void);
  void (* enable_interrupt)(void);
  tm_return_status_t init_status;
} TaskManager_t;

TaskManager_t task_manager = {0};

tm_return_status_t tm_init_task_manager(uint8_t capacity, void (* disable_interrupt)(void), void (* enable_interrupt)(void)) {
  disable_interrupt();
  task_manager.task_queue = q_create_queue();
  if (task_manager.task_queue == NULL) {
    task_manager.init_status = TM_INIT_FAILED;
    enable_interrupt();
    return TM_INIT_FAILED;
  }
  task_manager.task_list = (Task_t **)malloc(capacity * sizeof(task_manager.task_list));
  if (task_manager.task_list == NULL) {
    free(task_manager.task_queue);
    task_manager.init_status = TM_INIT_FAILED;
    enable_interrupt();
    return TM_INIT_FAILED;
  }
  for (uint8_t i = 0; i < capacity; i++) {
    task_manager.task_list[i] = NULL;
  }
  task_manager.capacity = capacity;
  task_manager.disable_interrupt = disable_interrupt;
  task_manager.enable_interrupt = enable_interrupt;
  task_manager.init_status = TM_INIT_SUCCEEDED;
  task_manager.enable_interrupt();
  return TM_INIT_SUCCEEDED;
}

void tm_sheduler(void) {
  if (task_manager.init_status == TM_INIT_FAILED) {
    return;
  }
  Task_t * task = NULL;
  if (task_manager.size == 0) {
    return;
  }
  for (uint8_t i = 0; i < task_manager.size; i++) {
    task = task_manager.task_list[i];
    if (task->delay) {
      --task->delay;
    }
    if (task->delay == 0) {
      q_push(task_manager.task_queue, task);
      task->delay = task->timeout;
    }
  }
}

void tm_task_manager(void) {
  if (task_manager.init_status == TM_INIT_FAILED) {
    return;
  }
  Task_t * task = NULL;
  while (!q_is_empty(task_manager.task_queue)) {
    task = (Task_t *)q_pop(task_manager.task_queue);
    task->task_function(task->context);
    if (task->run_mode == TM_ONE_SHOT_MODE) {
      tm_delete_task(task->task_function);
    }
  }
}

tm_return_status_t tm_create_task(void (* task_function)(void *), void * context, tm_task_run_mode_t run_mode, uint32_t delay, uint32_t timeout) {
  if (task_manager.init_status == TM_INIT_FAILED) {
    return TM_INIT_FAILED;
  }
  Task_t * task = NULL;
  if (task_manager.size == task_manager.capacity) {
    return TM_TASK_LIST_IS_FULL;
  }
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  task_manager.disable_interrupt();
  task = (Task_t *)malloc(sizeof(**task_manager.task_list));
  if (task == NULL) {
    task_manager.enable_interrupt();
    return TM_MALLOC_FAILED;
  }
  task->task_function = task_function;
  task->context = context;
  task->run_mode = run_mode;
  task->delay = delay;
  task->timeout = timeout;
  task_manager.task_list[task_manager.size] = task;
  task_manager.size++;
  task_manager.enable_interrupt();
  return TM_TASK_CREATED;
}

tm_return_status_t tm_delete_task(void (* task_function)(void *)) {
  if (task_manager.init_status == TM_INIT_FAILED) {
    return TM_INIT_FAILED;
  }
  Task_t * delete_task = NULL;
  Task_t * last_task = NULL;
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  task_manager.disable_interrupt();
  for (uint8_t i = 0; i < task_manager.size; i++) {
    if (task_manager.task_list[i]->task_function == task_function) {
      delete_task = task_manager.task_list[i];
      last_task = task_manager.task_list[task_manager.size - 1];
      task_manager.task_list[i] = last_task;
      task_manager.task_list[task_manager.size - 1] = NULL;
      free(delete_task);
      task_manager.size--;
      task_manager.enable_interrupt();
      return TM_TASK_DELETED;
    }
  }
  task_manager.enable_interrupt();
  return TM_TASK_NOT_FOUND;
}

tm_return_status_t tm_set_delay(void (* task_function)(void *), uint32_t delay) {
  if (task_manager.init_status == TM_INIT_FAILED) {
    return TM_INIT_FAILED;
  }
  Task_t * task = NULL;
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  task_manager.disable_interrupt();
  for (uint8_t i = 0; i < task_manager.size; i++) {
    task = task_manager.task_list[i];
    if (task->task_function == task_function) {
      task->delay = delay;
      task_manager.enable_interrupt();
      return TM_TASK_UPDATED;
    }
  }
  task_manager.enable_interrupt();
  return TM_TASK_NOT_FOUND;
}

tm_return_status_t tm_set_timeout(void (* task_function)(void *), uint32_t timeout) {
  if (task_manager.init_status == TM_INIT_FAILED) {
    return TM_INIT_FAILED;
  }
  Task_t * task = NULL;
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  task_manager.disable_interrupt();
  for (uint8_t i = 0; i < task_manager.size; i++) {
    task = task_manager.task_list[i];
    if (task->task_function == task_function) {
      task->timeout = timeout;
      task_manager.enable_interrupt();
      return TM_TASK_UPDATED;
    }
  }
  task_manager.enable_interrupt();
  return TM_TASK_NOT_FOUND;
}
