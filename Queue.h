#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>

typedef enum {
  Q_DATA_PUSHED,
  Q_DATA_DELETED,
  Q_MALLOC_FAILED,
  Q_QUEUE_LOST,
  Q_QUEUE_IS_EMPTY,
} q_return_status_t;

typedef struct Node {
  void * data;
  struct Node * next;
} QueueNode_t;

typedef struct {
  QueueNode_t * head;
  QueueNode_t * tail;
  uint16_t size;
} Queue_t;

Queue_t * q_create_queue(void);

q_return_status_t q_push(Queue_t * queue, void * data);

void * q_pop(Queue_t * queue);

void * q_get(Queue_t * queue);

q_return_status_t q_delete(Queue_t * queue);

int16_t q_get_size(Queue_t * queue);

int8_t q_is_empty(Queue_t * queue);

#endif
