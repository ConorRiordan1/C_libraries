/** @file threadpool.c
 *
 * @brief code for threadpool ops
 * code pulled from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 *
 * Author: Conor Riordan
 */
#include "threadpool.h"

/// @brief private p_work creation function
/// @param func function pointer with void args
/// @param p_args argument for the function requested - alloced struct works best
/// @return NULL on failure. p_work on success
/// @warning If any, args need to be alloced
/// @warning Does not add p_work to queue, should not be called out of context
static tpool_work_t * tpool_work_create(thread_func_t func_to_do, void * p_args) {

  tpool_work_t *p_work;

  if (func_to_do == NULL)
  {
    goto EXIT;
  }
  // p_args can be NULL if no args

  p_work = malloc(sizeof(*p_work));

  if (NULL == p_work)
  {
    goto EXIT;
  }

  p_work->func = func_to_do;
  p_work->arg = p_args;
  p_work->next = NULL;
  return p_work;

EXIT:
  return NULL;
}

/// @brief private p_work destroy function
/// @param p_work p_work to destroy
/// @warning do not call this function outside of context
static void tpool_work_destroy(tpool_work_t * p_work) {

  if (p_work == NULL)
  {
    return;
  }

  free(p_work);
}

/// @brief private get p_work function. Used by tpool
/// @param p_threadpool threadpool
/// @return p_work or NULL if no p_work
/// @warning do not call this function out of context
static tpool_work_t *tpool_work_get(tpool_t *p_threadpool) {
  tpool_work_t *p_work;

  if (p_threadpool == NULL)
  {
    goto EXIT;
  }

  p_work = p_threadpool->work_first;

  if (p_work == NULL)
  {
    goto EXIT;
  }


  if (p_work->next == NULL)
  {
    p_threadpool->work_first = NULL;
    p_threadpool->work_last = NULL;
  }
  else
  {
    p_threadpool->work_first = p_work->next;
  }

  return p_work;

EXIT:
  return NULL;
}

/// @brief private worker function
/// @param arg thread pool
/// @return nothing
static void *tpool_worker(void *arg) {
  tpool_t *p_threadpool = arg;
  tpool_work_t *p_work;

  for(;;)
  {
    pthread_mutex_lock(&(p_threadpool->work_mutex));

    while (p_threadpool->work_first == NULL && !p_threadpool->stop) // avoid spirious wakeups
    {
      pthread_cond_wait(&(p_threadpool->work_cond), &(p_threadpool->work_mutex));
    }

    if (p_threadpool->stop)
    {
      break;
    }

    p_work = tpool_work_get(p_threadpool); // critical section(for threadpool)
    p_threadpool->working_cnt++;
    pthread_mutex_unlock(&(p_threadpool->work_mutex));

    if (p_work != NULL)
    { // no longer critical for threadpool
      p_work->func(p_work->arg);
      tpool_work_destroy(p_work);
    }

    pthread_mutex_lock(&(p_threadpool->work_mutex));
    p_threadpool->working_cnt--;
    if (!p_threadpool->stop && p_threadpool->working_cnt == 0 &&
        p_threadpool->work_first == NULL)
        {
          pthread_cond_signal(&(p_threadpool->working_cond));
        } // critical section part 2
    pthread_mutex_unlock(&(p_threadpool->work_mutex));
  }

  // if break
  p_threadpool->thread_cnt--;
  pthread_cond_signal(&(p_threadpool->working_cond));
  pthread_mutex_unlock(&(p_threadpool->work_mutex));
  return NULL;
}

/// @brief public. create thread pool
/// @param num nmber of threads. 2 default
/// @return threadpool. NULL on failure
tpool_t *tpool_create(size_t num)
{

  tpool_t * p_threadpool;
  pthread_t thread;
  size_t i;

  // default is 2
  if (num == 0)
  {
    num = 2;
  }

  p_threadpool = calloc(1, sizeof(*p_threadpool));

  if (NULL == p_threadpool) {
    goto EXIT;
  }

  p_threadpool->thread_cnt = num;

  pthread_mutex_init(&(p_threadpool->work_mutex), NULL);
  printf("init\n");
  pthread_cond_init(&(p_threadpool->work_cond), NULL);
  pthread_cond_init(&(p_threadpool->working_cond), NULL);

  p_threadpool->work_first = NULL;
  p_threadpool->work_last = NULL;

  for (i = 0; i < num; i++) {
    pthread_create(&thread, NULL, tpool_worker, p_threadpool);
    pthread_detach(thread);
  }

  return p_threadpool;

EXIT:
  return NULL;
}

void tpool_destroy(tpool_t *p_threadpool) {
  tpool_work_t *p_work;
  tpool_work_t *p_work2;

  if (p_threadpool == NULL)
  {
    printf("Invalid tpool");
    return;
  }

  pthread_mutex_lock(&(p_threadpool->work_mutex));
  p_work = p_threadpool->work_first;

  while (p_work != NULL)
  {
    p_work2 = p_work->next;
    tpool_work_destroy(p_work);
    p_work = p_work2;
  }

  p_threadpool->stop = true;
  pthread_cond_broadcast(&(p_threadpool->work_cond));
  pthread_mutex_unlock(&(p_threadpool->work_mutex));

  tpool_wait(p_threadpool); // experiment with this

  pthread_mutex_destroy(&(p_threadpool->work_mutex));
  pthread_cond_destroy(&(p_threadpool->work_cond));
  pthread_cond_destroy(&(p_threadpool->working_cond));

  free(p_threadpool);
}

bool tpool_add_work(tpool_t *p_threadpool, thread_func_t func, void *arg) {
  tpool_work_t *p_work;

  if (p_threadpool == NULL)
  {
    return false;
  }

  p_work = tpool_work_create(func, arg);

  if (p_work == NULL)
  {
    return false;
  }

  pthread_mutex_lock(&(p_threadpool->work_mutex));

  if (p_threadpool->work_first == NULL)
  {
    p_threadpool->work_first = p_work;
    p_threadpool->work_last = p_threadpool->work_first;
  }
  else
  {
    p_threadpool->work_last->next = p_work;
    p_threadpool->work_last = p_work;
  }

  pthread_cond_broadcast(&(p_threadpool->work_cond));
  pthread_mutex_unlock(&(p_threadpool->work_mutex));

  return true;
}

void tpool_wait(tpool_t *p_threadpool) {
  if (p_threadpool == NULL)
  {
    return;
  }

  pthread_mutex_lock(&(p_threadpool->work_mutex));

  while (1)
  {
    if ((!p_threadpool->stop && p_threadpool->working_cnt != 0) ||
        (p_threadpool->stop && p_threadpool->thread_cnt != 0)) // should check if p_work in queue
    {
      pthread_cond_wait(&(p_threadpool->working_cond), &(p_threadpool->work_mutex));
    }
    else
    {
      break;
    }
  }
  pthread_mutex_unlock(&(p_threadpool->work_mutex));
}

// end of file
