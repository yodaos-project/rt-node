#ifndef _RTEV_THREADPOOL_H_
#define _RTEV_THREADPOOL_H_

#include "rtev.h"

typedef void (*threadpool_func)(void *data);

void threadpool_init(int size);

void threadpool_post_task(threadpool_func func, void *data);

#endif // _RTEV_THREADPOOL_H_