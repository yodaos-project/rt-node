/* Copyright 2018-present Samsung Electronics Co., Ltd. and other contributors
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

#ifndef JS_RTEV_WATCHER
#define JS_RTEV_WATCHER

#include "rt-node.h"

typedef void (*rtnode_close_cb)(rtev_watcher_t *);

typedef struct {
  jerry_value_t jobject;
  rtnode_close_cb close_cb;
} rtnode_rtev_watcher_data;

#define RTNODE_ALIGNUP(value, alignment) \
  (((value) + ((alignment)-1)) & ~((alignment)-1))

/**
 * Allocate and initialize an rtev_watcher_t structure with a jerry callback and
 * extra data.
 *
 * The allocated memory has the following layout:
 *
 *  |----------------|  <- start of rtev_watcher *
 *  | rtev_watcher_t |
 *  |                |
 *  |----------------|
 *  |    PADDING     |  <- alignment padding
 *  |----------------|  <- start of the js_rtev_watcher_data struct
 *  |  watcher_data  |
 *  |----------------|  <- start of the extra data if required
 *  |     extra      |
 *  |----------------|
 *
 */
void rtnode_rtev_watcher_bind(size_t watcher_size,
                              const jerry_value_t jobject,
                              const jerry_object_native_info_t *native,
                              size_t extra_data_size,
                              rtnode_close_cb close_cb);
int rtnode_rtev_watcher_close(rtev_watcher_t *watcher);

/**
 * Returns a pointer to the watcher data struct referenced
 * by the rtev_watcher_t->data member.
 */
#define RTNODE_RTEV_WATCHER_DATA(RTEV_WATCHER) \
  ((rtnode_rtev_watcher_data*)((RTEV_WATCHER)->data))

/**
 * Returns a char* pointer for any extra data.
 *
 * IMPORTANT!
 * Make sure that the extra data is correctly allocated by using the
 * js_rtev_watcher_bind method call.
 */
#define RTNODE_RTEV_WATCHER_EXTRA_DATA(RTEV_WATCHER) \
  ((char*)((char*)((RTEV_WATCHER)->data) + sizeof(rtnode_rtev_watcher_data)))


#endif /* JS_RTEV_WATCHER */
