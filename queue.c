/*
	Implementation by Groxx, with absolutely no guarantees, so don't complain to me if it breaks stuff.
	Feel free to use it for awesome, as needed.  Apply liberally, and induce vomiting if you ingest large doses.
	Note from #0d15eb: BREAKING CHANGE FROM #1d1057: this is now a generic storage, storing pointers to data.  Manage your memory accordingly.
	Note: now stores the "next" pointer prior to processing, allowing you to process and pop the first item in the list in one pass without losing your place in the queue (and possibly other shenanigans, this one was just handy for my uses so I changed it).
*/
#include <stdint.h>
#include <stdlib.h>
#include "queue.h"
#include <stdio.h>



void init_queue(struct queue_root* queue){
	queue->head = NULL;
	queue->tail = NULL;
}

void push_queue(queue_root* queue, int size, void * socket_num){
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


void * pop_queue(queue_root* queue){
	void * popped;
	if (queue->head == NULL){
		return NULL; // causes a compile warning.  Just check for ==NULL when popping.
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


/// @brief return 1 if empty, 0 if not empty
/// @param my_queue 
/// @return 
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
	while(pop_queue(my_queue) != NULL)
	{
		pop_queue(my_queue);
	}
}
