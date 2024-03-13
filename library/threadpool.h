/** @file threadpool.h
 *
 * @brief code for threadpool.c
 * code pulled from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 *
 * Author: Conor Riordan
 */
#ifndef __TPOOL_H__
#define __TPOOL_H__

#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct tpool;
typedef struct tpool tpool_t;

typedef void (*thread_func_t)(void *arg);

tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg);
void tpool_wait(tpool_t *tm);


typedef struct _tpool_work{
    thread_func_t func;
    void * arg;
    struct _tpool_work * next;
} tpool_work_t;


struct tpool {
    tpool_work_t    *work_first;
    tpool_work_t    *work_last;
    pthread_mutex_t  work_mutex;
    pthread_cond_t   work_cond;
    pthread_cond_t   working_cond; // 
    size_t           working_cnt;
    size_t           thread_cnt;
    bool             stop;
};

extern tpool_t * p_threadpool;

#endif /* __TPOOL_H__ */
