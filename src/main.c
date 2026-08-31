#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <math.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include "read.h"
#include "en_dc.h"
#include "read_file.h"
#include "drive.h"
#define NUM_PRODUCERS 1 
#define NUM_CONSUMERS 3 

Message_Queue queue;
Shared_Buffer shared_buffer;
ReadWrite_Lock lock;
pthread_mutex_t message_mutex;
pthread_cond_t message_available;
unsigned long message_generation=0;
int producer_finished=0;
// uint8_t input_msg[max_size]={10,20,30,40,50};
void *producer(void *arg)
{
    InputFile input;
    FileArgs *args = (FileArgs *)arg;

    if (input_file_open(&input, args->filename) != 0) {
        return NULL;
    }

    float x_coord;
    float y_coord;

    while (input_file_read(&input, &x_coord, &y_coord)) {

        Message msg = {0};

        /*
         * TODO:
         * Convert the coordinates into a transport message.
         */

        /*
         * TODO:
         * Store the message in the shared buffer safely.Make sure only one message is being stored to avoid duplicates
         * in the buffer. 
         */

        /*
         * TODO:
         * Notify waiting consumers.
         */
    }

    /*
     * TODO:
     * Notify consumers that production has finished.
     */

    return NULL;
}

void *consumer(void *arg)
{
    int id = *(int *)arg;

    /*
     * TODO:
     * Wait for a new message.
     */

    /*
     * TODO:
     * Safely retrieve the message from the
     * shared buffer.
     */

    /*
     * TODO:
     * Decode the message.
     */

    /*
     * TODO:
     * Forward the message to the drive queue.
     * make sure the message is shown only once to avoid duplicate entries .
     */

    return NULL;
}

void *drive_write(void *arg){
  int id=*(int *)arg;
  InputFile input;
  FileArgs *args=(FileArgs *) arg;
  if (input_file_open_write(&input, args->result_filename) != 0) {
    printf("Failed to open %s\n", args->result_filename);
    return NULL;
  }
  for(int i=0;i<10;i++){
    /* 
     * TODO : 
     * recieve the coordinate message 
     * contruct the targte coordinate 
     * maintain rover state
     * invoke the drive_to_target function in drive.c
     *
    */
    float dx=coordinate_target.latitude-rover.position.latitude;
    float dy=coordinate_target.longitude-rover.position.longitude;
    float error =hypotf(dx,dy);
    int status=1;
    if (result_status==DRIVE_REACHED_TARGET && error<=0.7){
      status=0;
    }
    if (result_status==DRIVE_REACHED_TARGET || error>0.07){
      status=1;
      break;
    }

    input_file_write(&input,&rover.position.latitude,&rover.position.longitude,&error,&status);

  }
  if(status ==0){
      printf("Success \n");
  }else {
      printf("Failed try again \n");
  }
  return NULL;
}


int main(){
  pthread_t producers[NUM_PRODUCERS];
  pthread_t consumers[NUM_CONSUMERS];
  pthread_t drive_writers[NUM_PRODUCERS];
  int producers_id[NUM_PRODUCERS] ={1,2,3};
  int consumer_id[NUM_CONSUMERS]={1,2,3};
  int writer_id[NUM_PRODUCERS]={1};
  const char *testcases[]={
    "input/testcase1.txt",
    "input/testcase2.txt",
    "input/testcase3.txt",
    "input/testcase4.txt"
  };
  const char *result_tc[]={
    "result/result1.txt",
    "result/result2.txt",
    "result/result3.txt",
    "result/result4.txt"
  };
  if(rwlock_init(&lock) !=0 ){
    printf("Reader writer synchrnization failed \n");
    return 1;
  }
  if(message_queue_init(&queue)!=0){
    printf("Queue Initialization failed \n");
    return 1;
  }
  if(pthread_cond_init(&message_available,NULL)!=0){
    printf("Condition mutex Initialization failed\n");
    return 1;

  }
  for(int i=0;i<4;i++){
    printf("Input : %d \n",i+1);
    printf("\n");
    printf("\n");
    FileArgs file_args={
      .id=1,
      .filename=testcases[i],
      .result_filename=result_tc[i]
    };
    message_generation=0;
    producer_finished=0;
    for(int i=0;i<NUM_PRODUCERS;i++){
      pthread_create(&producers[i],NULL,producer,&file_args);
    };
    for(int i=0;i<NUM_CONSUMERS;i++){
      pthread_create(&consumers[i],NULL,consumer,&consumer_id[i]);

    };

    for(int i = 0; i < NUM_PRODUCERS; i++) {
      pthread_create(
        &drive_writers[i],
        NULL,
        drive_write,
        &file_args
      );
    };
    for(int i=0;i<NUM_PRODUCERS;i++){
      pthread_join(producers[i],NULL);
    };
    for(int i=0;i<NUM_CONSUMERS;i++){
      pthread_join(consumers[i],NULL);
    };
    for (int i = 0; i < NUM_PRODUCERS; i++) {
      pthread_join(drive_writers[i], NULL);
    };
 

  };
  rwlock_destroy(&lock);
  message_destroy(&queue);


}
cmake -S . -B build

cmake --build build --verbose

./build/queue_test
