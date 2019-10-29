#include "threadpool.h"
#include "queue.h"

typedef enum {
  RUNNING = 0,
  STOPPED = 1,
} threadpool_state;

typedef struct {
  threadpool_func func;
  void *func_data;
  QUEUE node;
} threadpool_task;

typedef struct {
  pthread_t thread;
  int id;
} threadpool_runner;

static threadpool_runner *task_runners = NULL;
static int threadpool_size = 0;
static QUEUE task_queue;
static int task_size = 0;
static pthread_mutex_t task_lock;
static pthread_cond_t task_cond;
static threadpool_state task_state;

#define NEW_TASK \
  if (!QUEUE_EMPTY(&task_queue)) { \
    QUEUE *head = QUEUE_HEAD(&task_queue); \
    QUEUE_REMOVE(head); \
    QUEUE_INIT(head); \
    task = QUEUE_DATA(head, threadpool_task, node); \
    --task_size; \
  }

static void* threadpool_run_task(void *data) {
  threadpool_runner *runner = (threadpool_runner *) data;
  threadpool_task *task = NULL;
  pthread_mutex_lock(&task_lock);
  while (true) {
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
    task->func(task->func_data);
    free(task);
    task = NULL;
    pthread_mutex_lock(&task_lock);
    NEW_TASK;
  }
  rtev_free(runner);
  --threadpool_size;
  pthread_exit(NULL);
  return NULL;
}

#undef NEW_TASK

void threadpool_init(int size) {
  task_state = RUNNING;
  threadpool_size = size;
  QUEUE_INIT(&task_queue);
  size_t threadpool_bytes = sizeof(threadpool_runner) * size;
  task_runners = rtev_malloc(threadpool_bytes);
  memset(task_runners, 0, threadpool_bytes);
  pthread_mutex_init(&task_lock, NULL);
  pthread_cond_init(&task_cond, NULL);
  for (int i = 0; i < size; ++i) {
    threadpool_runner *runner = task_runners + i;
    runner->id = i;
    pthread_create(&runner->thread, NULL, threadpool_run_task, runner);
    pthread_detach(runner->thread);
  }
}

void threadpool_post_task(threadpool_func func, void *data) {
  threadpool_task *task = rtev_malloc(sizeof(threadpool_task));
  memset(task, 0, sizeof(threadpool_task));
  task->func = func;
  task->func_data = data;
  pthread_mutex_lock(&task_lock);
  QUEUE_INIT(&task->node);
  QUEUE_INSERT_TAIL(&task_queue, &task->node);
  ++task_size;
  pthread_cond_signal(&task_cond);
  pthread_mutex_unlock(&task_lock);
}

void threadpool_stop() {
  pthread_mutex_lock(&task_lock);
  task_state = STOPPED;
  pthread_cond_signal(&task_cond);
  pthread_mutex_unlock(&task_lock);
}