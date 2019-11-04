#include "rtev.h"

typedef enum {
  RUNNING = 0,
  STOPPED = 1,
} threadpool_state;

typedef struct {
  rtev_threadpool_fn fn;
  void *data;
  QUEUE node;
} threadpool_task;

typedef struct {
  pthread_t thread;
  int id;
} threadpool_runner;

#ifndef RTEV_THREADPOOL_SIZE
#define RTEV_THREADPOOL_SIZE 1
#endif
static int threadpool_size = RTEV_THREADPOOL_SIZE;
static threadpool_runner task_runners[RTEV_THREADPOOL_SIZE];
static QUEUE task_queue;
static int task_size = 0;
static pthread_mutex_t task_lock;
static pthread_cond_t task_cond;
static threadpool_state task_state;

#define NEW_TASK                                      \
  if (!QUEUE_EMPTY(&task_queue)) {                    \
    QUEUE *head = QUEUE_HEAD(&task_queue);            \
    QUEUE_REMOVE(head);                               \
    QUEUE_INIT(head);                                 \
    task = QUEUE_DATA(head, threadpool_task, node);   \
    --task_size;                                      \
  }

static void* rtev_threadpool_run_task(void *data) {
  threadpool_task *task = NULL;
  pthread_mutex_lock(&task_lock);
  while (true) {
    NEW_TASK;
    if (task == NULL) {
      pthread_cond_wait(&task_cond, &task_lock);
      if (task_state == STOPPED) {
        pthread_cond_signal(&task_cond);
        pthread_mutex_unlock(&task_lock);
        break;
      }
      NEW_TASK;
    }
    pthread_mutex_unlock(&task_lock);
    if (task == NULL) {
      continue;
    }
    task->fn(task->data);
    rtev_free(task);
    task = NULL;
    pthread_mutex_lock(&task_lock);
  }
  --threadpool_size;
  return NULL;
}

#undef NEW_TASK

void _rtev_threadpool_init() {
  task_state = RUNNING;
  QUEUE_INIT(&task_queue);
  pthread_mutex_init(&task_lock, NULL);
  pthread_cond_init(&task_cond, NULL);
  for (int i = 0; i < threadpool_size; ++i) {
    threadpool_runner *runner = task_runners + i;
    runner->id = i;
    pthread_create(&runner->thread, NULL, rtev_threadpool_run_task, runner);
    pthread_detach(runner->thread);
  }
}

void rtev_threadpool_post(rtev_threadpool_fn fn, void *data) {
  threadpool_task *task = rtev_malloc(sizeof(threadpool_task));
  memset(task, 0, sizeof(threadpool_task));
  task->fn = fn;
  task->data = data;
  pthread_mutex_lock(&task_lock);
  QUEUE_INIT(&task->node);
  QUEUE_INSERT_TAIL(&task_queue, &task->node);
  ++task_size;
  pthread_cond_signal(&task_cond);
  pthread_mutex_unlock(&task_lock);
}

void rtev_threadpool_stop() {
  pthread_mutex_lock(&task_lock);
  task_state = STOPPED;
  pthread_cond_signal(&task_cond);
  pthread_mutex_unlock(&task_lock);
}