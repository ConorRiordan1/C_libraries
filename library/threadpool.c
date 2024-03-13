/** @file threadpool.c
 *
 * @brief code for threadpool ops
 * code pulled from https://nachtimwald.com/2019/04/12/thread-pool-in-c/
 *
 * Author: Conor Riordan
 */
#include "threadpool.h"

/// @brief private work creation function
/// @param func function pointer with void args
/// @param p_args argument for the function requested - alloced struct works best
/// @return NULL on failure. Work on success
/// @warning If any, args need to be alloced
/// @warning Does not add work to queue, should not be called out of context
static tpool_work_t * tpool_work_create(thread_func_t func_to_do, void * p_args) {

  tpool_work_t *work;

  if (func_to_do == NULL)
  {
    goto EXIT;
  }
  // p_args can be NULL if no args

  work = malloc(sizeof(*work));

  if (NULL == work)
  {
    goto EXIT;
  }

  work->func = func_to_do;
  work->arg = p_args;
  work->next = NULL;
  return work;

EXIT:
  return NULL;
}

/// @brief private work destroy function
/// @param work work to destroy
/// @warning do not call this function outside of context
static void tpool_work_destroy(tpool_work_t * p_work) {

  if (p_work == NULL)
  {
    return;
  }

  free(p_work);
}

/// @brief private get work function. Used by tpool
/// @param tm threadpool
/// @return work or NULL if no work
/// @warning do not call this function out of context
static tpool_work_t *tpool_work_get(tpool_t *p_tm) {
  tpool_work_t *work;

  if (p_tm == NULL)
  {
    goto EXIT;
  }

  work = p_tm->work_first;

  if (work == NULL)
  {
    goto EXIT;
  }


  if (work->next == NULL) {
    tm->work_first = NULL;
    tm->work_last = NULL;
  } else {
    tm->work_first = work->next;
  }

  return work;

EXIT:
  return NULL;
}

/// @brief private worker function
/// @param arg thread pool
/// @return nothing
static void *tpool_worker(void *arg) {
  tpool_t *tm = arg;
  tpool_work_t *work;

  while (1) {
    pthread_mutex_lock(&(tm->work_mutex));

    while (tm->work_first == NULL && !tm->stop) // avoid spirious wakeups
    {
      pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));
    }

    if (tm->stop)
      break;

    work = tpool_work_get(tm); // critical section(for threadpool)
    tm->working_cnt++;
    pthread_mutex_unlock(&(tm->work_mutex));

    if (work != NULL) { // no longer critical for threadpool
      work->func(work->arg);
      tpool_work_destroy(work);
    }

    pthread_mutex_lock(&(tm->work_mutex));
    tm->working_cnt--;
    if (!tm->stop && tm->working_cnt == 0 &&
        tm->work_first == NULL) // critical section part 2
      pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
  }

  tm->thread_cnt--;
  pthread_cond_signal(&(tm->working_cond));
  pthread_mutex_unlock(&(tm->work_mutex));
  return NULL;
}

/// @brief public. create thread pool
/// @param num nmber of threads. 2 default
/// @return threadpool. NULL on failure
tpool_t *tpool_create(size_t num) {

  tpool_t *tm;
  pthread_t thread;
  size_t i;

  if (num == 0)
    num = 2;

  tm = calloc(1, sizeof(*tm));

  if (NULL == tm) {
    goto EXIT;
  }

  tm->thread_cnt = num;

  pthread_mutex_init(&(tm->work_mutex), NULL);
  printf("init\n");
  pthread_cond_init(&(tm->work_cond), NULL);
  pthread_cond_init(&(tm->working_cond), NULL);

  tm->work_first = NULL;
  tm->work_last = NULL;

  for (i = 0; i < num; i++) {
    pthread_create(&thread, NULL, tpool_worker, tm);
    pthread_detach(thread);
  }

  return tm;

EXIT:
  return NULL;
}

void tpool_destroy(tpool_t *tm) {
  tpool_work_t *work;
  tpool_work_t *work2;

  if (tm == NULL) {
    printf("Invalid tpool");
    return;
  }

  pthread_mutex_lock(&(tm->work_mutex));
  work = tm->work_first;
  while (work != NULL) {
    work2 = work->next;
    tpool_work_destroy(work);
    work = work2;
  }
  tm->stop = true;
  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));

  tpool_wait(tm); // experiment with this

  pthread_mutex_destroy(&(tm->work_mutex));
  pthread_cond_destroy(&(tm->work_cond));
  pthread_cond_destroy(&(tm->working_cond));

  free(tm);
}

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg) {
  tpool_work_t *work;

  if (tm == NULL)
    return false;

  work = tpool_work_create(func, arg);
  if (work == NULL)
    return false;

  pthread_mutex_lock(&(tm->work_mutex));
  if (tm->work_first == NULL) {
    tm->work_first = work;
    tm->work_last = tm->work_first;
  } else {
    tm->work_last->next = work;
    tm->work_last = work;
  }

  pthread_cond_broadcast(&(tm->work_cond));
  pthread_mutex_unlock(&(tm->work_mutex));

  return true;
}

void tpool_wait(tpool_t *tm) {
  if (tm == NULL)
    return;

  pthread_mutex_lock(&(tm->work_mutex));
  while (1) {
    if ((!tm->stop && tm->working_cnt != 0) ||
        (tm->stop && tm->thread_cnt != 0)) // should check if work in queue
    {
      pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
    } else {
      break;
    }
  }
  pthread_mutex_unlock(&(tm->work_mutex));
}

// end of file