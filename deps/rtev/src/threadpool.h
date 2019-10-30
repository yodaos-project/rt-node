#ifndef _RTEV_THREADPOOL_H_
#define _RTEV_THREADPOOL_H_

#include "rtev.h"

typedef void (*rtev_threadpool_func)(void *data);

void rtev_threadpool_init(int size);

void rtev_threadpool_post(rtev_threadpool_func func, void *data);

#endif // _RTEV_THREADPOOL_H_