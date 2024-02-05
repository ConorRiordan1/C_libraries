#ifndef QUEUE_H
#define QUEUE_H
#include <openssl/ssl.h>
#include <openssl/err.h>
typedef struct queue_item {
	void * socket_num;
	struct queue_item * next;
}queue_item;


typedef struct queue_root {
	queue_item* head;
	queue_item* tail;
}queue_root;


int is_empty(queue_root * my_queue);
void * pop_queue(queue_root* queue);
void push_queue(queue_root* queue, int size, void * socket_num);
void init_queue(struct queue_root* queue);
void free_queue(queue_root * my_queue);


#endif