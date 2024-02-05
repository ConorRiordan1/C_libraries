#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "example.h"
#include <unistd.h>



queue_root my_queue;
pthread_mutex_t queue_mutex;
tpool_t *tm;



void add_to_queue(void * arg)
{   
    printf("thread id: %ld\n",pthread_self());
    fflush(stdout);
    pthread_mutex_lock(&queue_mutex);
    push_queue(&my_queue,sizeof(int),*(int *) arg);
    pthread_mutex_unlock(&queue_mutex);
    sleep(.1);
}


int push_sockets( SSL * socket) // old main
{

    //printf(" I am push socket, I got this %d\n",socket);
    printf("in example\n");
    char buffer[64];

    int bytes_recv = SSL_read(socket, buffer, 64);

    if (bytes_recv > 0) {
        buffer[bytes_recv] = '\0'; // Null-terminate the received data
        printf("I got: %s\n", buffer);
    } else {
        // Handle error or connection closure
        printf("Error in SSL_read\n");
    }


    
    init_queue(&my_queue);

    tpool_t *tm;

    tm = tpool_create(3);
    int * vals;
    int num_of_items = 100;
    pthread_mutex_init(&queue_mutex,NULL);


    vals = calloc(num_of_items,sizeof(*vals));

    for(int idx = 0; idx<num_of_items;idx++)
    {
        vals[idx] = idx;
        tpool_add_work(tm,add_to_queue,vals+idx);
    }

    tpool_wait(tm);
    

    for(int idx = 0; idx<num_of_items;idx++)
    {
        pthread_mutex_lock(&queue_mutex);
        printf("this%d\n",pop_queue(&my_queue));
        pthread_mutex_unlock(&queue_mutex);
    }

    free(vals);
    tpool_destroy(tm);
    pthread_mutex_destroy(&queue_mutex);
    printf("done\n");

    return 0;











}

int init_my_stuff()
{
    init_queue(&my_queue);

    tm = tpool_create(3);
    pthread_mutex_init(&queue_mutex,NULL);
}