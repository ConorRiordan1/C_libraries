#ifndef QUEUE_H
#define QUEUE_H

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


#endif