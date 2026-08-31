#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#include "read.h"

int rwlock_init(ReadWrite_Lock *rw){
    rw->reader = 0;

    if (pthread_mutex_init(&rw->reader_count, NULL) != 0)
        return -1;

    if (pthread_mutex_init(&rw->writer_count, NULL) != 0) {
        pthread_mutex_destroy(&rw->reader_count);
        return -1;
    }

    if (sem_init(&rw->resource, 0, 1) != 0) {
        pthread_mutex_destroy(&rw->reader_count);
        pthread_mutex_destroy(&rw->writer_count);
        return -1;
    }

    return 0;
}

/*
 * Reader Entry
 *
 * TODO:
 * - Implement the reader-side synchronization logic.
 * - Multiple readers should be able to access the shared resource
 *   concurrently.
 * - The first reader must ensure that a writer cannot access the
 *   resource while readers are active.
 * - Ensure reader_count is modified safely.
 * - Ensure all acquired synchronization primitives are released
 *   correctly.
 */
void reader_enter(ReadWrite_Lock *lock){
    pthread_mutex_lock(&lock->writer_count);
    pthread_mutex_lock(&lock->reader_count);
    lock->reader++;
    if (lock->reader == 1) {           // was: != 1  (inverted)
        sem_wait(&lock->resource);     // was: missing entirely
    }
    pthread_mutex_unlock(&lock->reader_count);
    pthread_mutex_unlock(&lock->writer_count);
}

/*
 * Reader Exit
 *
 * TODO:
 * - Implement the reader exit logic.
 * - Decrement the active reader count safely.
 * - Ensure the resource becomes available to writers when
 *   the last reader exits.
 */
void reader_exit(ReadWrite_Lock *rw){
    pthread_mutex_lock(&rw->reader_count);

    rw->reader--;

    if(rw->reader == 0){
        sem_post(&rw->resource);
    }

    pthread_mutex_unlock(&rw->reader_count);
}

/*
 * Writer Entry
 *
 * TODO:
 * - Ensure writers obtain exclusive access to the shared resource.
 * - Prevent writers from accessing the resource while readers
 *   are active.
 * - Ensure writer synchronization is handled correctly.
 */
void writer_enter(ReadWrite_Lock *lock){
    pthread_mutex_lock(&lock->writer_count);
    sem_wait(&lock->resource);
}

/*
 * Writer Exit
 *
 * TODO:
 * - Release the shared resource.
 * - Release any synchronization primitive acquired by writer_enter().
 */
void writer_exit(ReadWrite_Lock *lock){
    sem_post(&lock->resource);
    pthread_mutex_unlock(&lock->writer_count);
}

void rwlock_destroy(ReadWrite_Lock *rw)
{
    pthread_mutex_destroy(&rw->reader_count);
    pthread_mutex_destroy(&rw->writer_count);
    sem_destroy(&rw->resource);
}
