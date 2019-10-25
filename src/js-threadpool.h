#ifndef _JS_THREADPOOL_H
#define _JS_THREADPOOL_H

#include "js-common.h"

typedef void (*threadpool_func)(void *data);

void threadpool_init(int size);

void threadpool_post_task(threadpool_func func, void *data);

#endif // _JS_THREADPOOL_H