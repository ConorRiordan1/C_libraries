#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


/// Until next comments just stuff to create queues

typedef struct queue_item {
	int socket_num;
	struct queue_item * next;
}queue_item;


typedef struct queue_root {
	queue_item* head;
	queue_item* tail;
}queue_root;


int is_empty(queue_root * my_queue);
int pop_queue(queue_root* queue);
void push_queue(queue_root* queue, int size, int socket_num);
void init_queue(struct queue_root* queue);
void free_queue(queue_root * my_queue);

void init_queue(struct queue_root* queue){
	queue->head = NULL;
	queue->tail = NULL;
}

void push_queue(queue_root* queue, int size, int socket_num){
	queue_item *item = malloc(sizeof(queue_item));//change depending on type
	item->socket_num = socket_num;
	item->next = NULL;
	if (queue->head == NULL){
		queue->head = item;
		queue->tail = item;
	} else {
		queue->tail->next = item;
		queue->tail = item;
	}
}


int  pop_queue(queue_root* queue){
	int popped;
	if (queue->head == NULL){
		return 0; // causes a compile warning.  Just check for ==NULL when popping.
	} else {
		popped = queue->head->socket_num;
		struct queue_item* next = queue->head->next;
		free(queue->head);
		queue->head = next;
		if (queue->head == NULL)
			queue->tail = NULL;
	}
	return popped;
}


int is_empty(queue_root * my_queue)
{
	if (my_queue->head == NULL)
	{
		return 1;
	}
	else{
		return 0;
	}
}

void free_queue(queue_root * my_queue)
{
	while(pop_queue(my_queue) != 0)
	{
		pop_queue(my_queue);
	}
}


/// END OF QUEUE FUNCTIONS


pthread_t tid;
pthread_mutex_t mutex1;
pthread_cond_t cond1;
queue_root my_queue;


static void cleanup_handler(void * arg)
{
	pthread_mutex_unlock(&mutex1);
}

static void * _atm_callback()
{

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);

	pthread_cleanup_push(cleanup_handler,NULL);

    for(;;)
    {
    	pthread_mutex_lock(&mutex1);

    	while(is_empty(&my_queue) == 1)// use function to check if q empty instead
    	{
        	pthread_testcancel();
        	pthread_cond_wait(&cond1,&mutex1);

    	}

		printf("Tid is popping from queue!\n");
		pop_queue(&my_queue);

		pthread_mutex_unlock(&mutex1);


	}
	
	pthread_cleanup_pop(1);

}

int main()
{

    init_queue(&my_queue);
    pthread_mutex_init(&mutex1,NULL);
    pthread_cond_init(&cond1,NULL);

    if(pthread_create(&tid, NULL, _atm_callback, NULL))
    {
        perror("pthread create error");
        goto EXIT;
    }

	for(int idx = 0;idx<10;idx++)
	{
		int test_var = 1;

		pthread_mutex_lock(&mutex1);

		push_queue(&my_queue,sizeof(int),test_var);
		printf("main thread pushed to queue!\n");

		pthread_cond_signal(&cond1);

		pthread_mutex_unlock(&mutex1);

	}
	printf("main outside!\n");


	pthread_cancel(tid);
	pthread_join(tid,NULL);

	free_queue(&my_queue);
	pthread_mutex_destroy(&mutex1);
	pthread_cond_destroy(&cond1);



	EXIT:
		return 1;
}