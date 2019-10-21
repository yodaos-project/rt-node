/* Copyright 2017-present Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef UV_FREERTOS_H
#define UV_FREERTOS_H

#include <sys/select.h>
#include <unistd.h>
#include <errno.h>

#include <netdb.h>

#ifndef TUV_POLL_EVENTS_SIZE
#define TUV_POLL_EVENTS_SIZE  32
#endif

// TUV_CHANGES@20161130: FIXME: What is the reasonable nubmer?
#ifndef IOV_MAX
#define IOV_MAX TUV_POLL_EVENTS_SIZE
#endif

#define SIGPROF       27

//-----------------------------------------------------------------------------
// thread and mutex
#define UV_PLATFORM_RWLOCK_T pthread_mutex_t

//-----------------------------------------------------------------------------
// uio
ssize_t readv(int __fd, const struct iovec* __iovec, int __count);
ssize_t writev(int __fd, const struct iovec* __iovec, int __count);

#define POLLIN      (1u << 0)      /* data other than high-priority may be read without blocking */
#define POLLRDNORM  (1u << 1)      /* normal data may be read without blocking */
#define POLLRDBAND  (1u << 2)      /* priority data may be read without blocking */
#define POLLPRI     (POLLRDBAND)   /* high-priority data may be read without blocking */
// Note: POLLPRI is made equivalent to POLLRDBAND in order to fit all these events into one byte
#define POLLOUT     (1u << 3)      /* normal data may be written without blocking */
#define POLLWRNORM  (POLLOUT)      /* equivalent to POLLOUT */
#define POLLWRBAND  (1u << 4)      /* priority data my be written */
#define POLLERR     (1u << 5)      /* some poll error occurred */
#define POLLHUP     (1u << 6)      /* file descriptor was "hung up" */
#define POLLNVAL    (1u << 7)      /* the specified file descriptor is invalid */

#ifdef __cplusplus
extern "C" {
#endif

struct pollfd {
    int fd;        /* The descriptor. */
    short events;  /* The event(s) is/are specified here. */
    short revents; /* Events found are returned here. */
};

typedef unsigned int nfds_t;

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

//-----------------------------------------------------------------------------
// etc
int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
//-----------------------------------------------------------------------------
// structure extension for nuttx                                                          
//                                                                                        
#define UV_PLATFORM_LOOP_FIELDS                                               \
  struct pollfd pollfds[TUV_POLL_EVENTS_SIZE];                                \
  int npollfds;                                                               \



#endif // UV_FREERTOS_H
