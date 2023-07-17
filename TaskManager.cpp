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

typedef enum {
  MUTEX_BUSY,
  MUTEX_FREE
} mutex_t;

typedef struct {
  Task_t ** task_list;
  Queue_t * task_exe_queue;
  uint8_t capacity;
  uint8_t task_number;
  mutex_t volatile mutex;
  tm_return_status_t init_status;
} TaskManager_t;

TaskManager_t task_manager = {0};

tm_return_status_t tm_init_task_manager(uint8_t capacity) {
  task_manager.task_exe_queue = q_create_queue();
  if (task_manager.task_exe_queue == NULL) {
    task_manager.init_status = TM_INIT_FAILED;
    return TM_INIT_FAILED;
  }
  task_manager.task_list = (Task_t **)malloc(capacity * sizeof(task_manager.task_list));
  if (task_manager.task_list == NULL) {
    free(task_manager.task_exe_queue);
    task_manager.init_status = TM_INIT_FAILED;
    return TM_INIT_FAILED;
  }
  for (uint8_t i = 0; i < capacity; i++) {
    task_manager.task_list[i] = NULL;
  }
  task_manager.capacity = capacity;
  task_manager.task_number = 0;
  task_manager.mutex = MUTEX_FREE;
  task_manager.init_status = TM_INIT_SUCCEEDED;
  return TM_INIT_SUCCEEDED;
}

void set_mutex_state(mutex_t state) {
  task_manager.mutex = state;
}

uint8_t mutex_is_busy(void) {
  if (task_manager.mutex == MUTEX_BUSY) {
    return 1;
  }
  return 0;
}

void tm_sheduler(void) {
  if (task_manager.init_status != TM_INIT_SUCCEEDED) {
    return;
  }
  if (mutex_is_busy()) {
    return;
  }
  Task_t * task = NULL;
  for (uint8_t i = 0; i < task_manager.task_number; i++) {
    task = task_manager.task_list[i];
    if (task->delay) {
      --task->delay;
    }
    if (task->delay == 0) {
      task->delay = task->timeout;
      q_push(task_manager.task_exe_queue, task);
    }
  }
}

void tm_task_manager(void) {
  if (task_manager.init_status == TM_INIT_FAILED) {
    return;
  }
  Task_t * task = NULL;
  while (!q_is_empty(task_manager.task_exe_queue)) {
    task = (Task_t *)q_pop(task_manager.task_exe_queue);
    task->task_function(task->context);
    if (task->run_mode == TM_ONE_SHOT_MODE) {
      tm_delete_task(task->task_function);
    }
  }
}

tm_return_status_t tm_create_task(void (* task_function)(void *), void * context, tm_task_run_mode_t run_mode, uint32_t delay, uint32_t timeout) {
  if (task_manager.init_status != TM_INIT_SUCCEEDED) {
    return TM_INIT_FAILED;
  }
  if (task_manager.task_number == task_manager.capacity) {
    return TM_TASK_LIST_IS_FULL;
  }
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  Task_t * task = (Task_t *)malloc(sizeof(**task_manager.task_list));
  if (task == NULL) {
    return TM_MALLOC_FAILED;
  }
  task->task_function = task_function;
  task->context = context;
  task->run_mode = run_mode;
  task->delay = delay;
  task->timeout = timeout;
  set_mutex_state(MUTEX_BUSY);
  task_manager.task_list[task_manager.task_number] = task;
  ++task_manager.task_number;
  set_mutex_state(MUTEX_FREE);
  return TM_TASK_CREATED;
}

tm_return_status_t tm_delete_task(void (* task_function)(void *)) {
  if (task_manager.init_status != TM_INIT_SUCCEEDED) {
    return TM_INIT_FAILED;
  }
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  Task_t * task = NULL;
  Task_t * task_from_queue = NULL;
  uint8_t task_queue_size = 0;
  set_mutex_state(MUTEX_BUSY);
  for (uint8_t i = 0; i < task_manager.task_number; i++) {
    task = task_manager.task_list[i];
    if (task->task_function == task_function) {
      task_queue_size = q_get_size(task_manager.task_exe_queue);
      for (uint8_t j = 0; j < task_queue_size; j++) {
        task_from_queue = (Task_t *)q_pop(task_manager.task_exe_queue);
        if (task_from_queue == task) {
          break;
        }
        q_push(task_manager.task_exe_queue, task_from_queue);
      }
      task_manager.task_list[i] = task_manager.task_list[task_manager.task_number - 1];
      task_manager.task_list[task_manager.task_number - 1] = NULL;
      free(task);
      --task_manager.task_number;
      set_mutex_state(MUTEX_FREE);
      return TM_TASK_DELETED;
    }
  }
  set_mutex_state(MUTEX_FREE);
  return TM_TASK_NOT_FOUND;
}

tm_return_status_t tm_set_delay(void (* task_function)(void *), uint32_t delay) {
  if (task_manager.init_status != TM_INIT_SUCCEEDED) {
    return TM_INIT_FAILED;
  }
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  Task_t * task = NULL;
  set_mutex_state(MUTEX_BUSY);
  for (uint8_t i = 0; i < task_manager.task_number; i++) {
    task = task_manager.task_list[i];
    if (task->task_function == task_function) {
      task->delay = delay;
      set_mutex_state(MUTEX_FREE);
      return TM_TASK_UPDATED;
    }
  }
  set_mutex_state(MUTEX_FREE);
  return TM_TASK_NOT_FOUND;
}

tm_return_status_t tm_set_timeout(void (* task_function)(void *), uint32_t timeout) {
  if (task_manager.init_status != TM_INIT_SUCCEEDED) {
    return TM_INIT_FAILED;
  }
  if (task_function == NULL) {
    return TM_TASK_FUNCTION_IS_EMPTY;
  }
  Task_t * task = NULL;
  set_mutex_state(MUTEX_BUSY);
  for (uint8_t i = 0; i < task_manager.task_number; i++) {
    task = task_manager.task_list[i];
    if (task->task_function == task_function) {
      task->timeout = timeout;
      set_mutex_state(MUTEX_FREE);
      return TM_TASK_UPDATED;
    }
  }
  set_mutex_state(MUTEX_FREE);
  return TM_TASK_NOT_FOUND;
}

uint8_t get_task_number(void) {
  return task_manager.task_number;
}
