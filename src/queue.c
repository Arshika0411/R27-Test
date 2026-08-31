#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "read.h"

static int sem_wait_retry(sem_t *sem) {
  int rc;
  do {
    rc = sem_wait(sem);
  } while (rc != 0 && errno == EINTR);
  return rc;
}

int message_queue_push(Message_Queue *queue, const Message *msg) {
  if (sem_wait_retry(&queue->empty) != 0) {
    return -1;
  }

  pthread_mutex_lock(&queue->mutex);
  queue->buffer[queue->tail] = *msg;
  queue->tail = (queue->tail + 1) % 50; // do i have to make changes in the read.h file too??
  pthread_mutex_unlock(&queue->mutex);

  sem_post(&queue->full);
  return 0;
}

int message_queue_pop(Message_Queue *queue, Message *msg) {
  if (sem_wait_retry(&queue->full) != 0) {
    return -1;
  }

  pthread_mutex_lock(&queue->mutex);
  *msg = queue->buffer[queue->head];
  queue->head = (queue->head + 1) % 50;
  pthread_mutex_unlock(&queue->mutex);

  sem_post(&queue->empty);
  return 0;
}




