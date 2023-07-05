#include "Queue.h"
#include <stdlib.h>

Queue_t * q_create_queue(void) {
  Queue_t * queue = (Queue_t *)malloc(sizeof(*queue));
  if (queue == NULL) {
    return NULL;
  }
  queue->head = NULL;
  queue->tail = NULL;
  queue->size = 0;
  return queue;
}

QueueNode_t * create_node(void * data) {
  QueueNode_t * new_node = (QueueNode_t *)malloc(sizeof(*new_node));
  if (new_node == NULL) {
    return NULL;
  }
  new_node->data = data;
  new_node->next = NULL;
  return new_node;
}

void delete_node(QueueNode_t * node) {
  free(node);
}

q_return_status_t q_push(Queue_t * queue, void * data) {
  if (queue == NULL) {
    return Q_QUEUE_LOST;
  }
  QueueNode_t * new_node = create_node(data);
  if (new_node == NULL) {
    return Q_MALLOC_FAILED;
  }
  if (queue->tail == NULL) {
    queue->tail = new_node;
    queue->head = new_node;
    ++queue->size;
    return Q_DATA_PUSHED;
  }
  queue->tail->next = new_node;
  queue->tail = new_node;
  ++queue->size;
  return Q_DATA_PUSHED;
}

void * q_pop(Queue_t * queue) {
  if (queue == NULL) {
    return NULL;
  }
  void * data = NULL;
  QueueNode_t * node = NULL;
  if (queue->head == NULL) {
    return NULL;
  }
  data = queue->head->data;
  if (queue->head == queue->tail) {
    delete_node(queue->head);
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    return data;
  }
  node = queue->head;
  queue->head = queue->head->next;
  delete_node(node);
  --queue->size;
  return data;
}

void * q_get(Queue_t * queue) {
  if (queue == NULL) {
    return NULL;
  }
  void * data = NULL;
  if (queue->head == NULL) {
    return NULL;
  }
  data = queue->head->data;
  return data;
}

q_return_status_t q_delete(Queue_t * queue) {
  if (queue == NULL) {
    return Q_QUEUE_LOST;
  }
  void * data = NULL;
  if (queue->head == NULL) {
    return Q_QUEUE_IS_EMPTY;
  }
  data = q_pop(queue);
  return Q_DATA_DELETED;
}

int16_t q_get_size(Queue_t * queue) {
  if (queue == NULL) {
    return -1;
  }
  return queue->size;
}

int8_t q_is_empty(Queue_t * queue) {
  if (queue == NULL) {
    return -1;
  }
  if (queue->size == 0) {
    return 1;
  }
  return 0;
}
