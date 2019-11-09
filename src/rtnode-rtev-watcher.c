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

#include "rtnode-rtev-watcher.h"

static void js_rtev_watcher_close_processor(rtev_watcher_t *watcher) {
  rtnode_rtev_watcher_data *watcher_data = RTNODE_RTEV_WATCHER_DATA(watcher);

  if (watcher_data->close_cb != NULL) {
    watcher_data->close_cb(watcher);
  }

  // Decrease ref count of Javascript object. From now the object can be
  // reclaimed.
  jerry_release_value(watcher_data->jobject);
  rtnode_free(watcher);
}

void rtnode_rtev_watcher_bind(size_t watcher_size,
                              const jerry_value_t jobject,
                              const jerry_object_native_info_t *native,
                              size_t extra_data_size,
                              rtnode_close_cb close_cb) {
  RTNODE_ASSERT(jerry_value_is_object(jobject));

  /* Make sure that the jerry_value_t is aligned */
  size_t aligned_request_size = RTNODE_ALIGNUP(watcher_size, 8u);

  char* request_memory = rtnode_malloc(
    aligned_request_size + sizeof(rtnode_rtev_watcher_data) + extra_data_size);
  rtev_watcher_t *watcher = (rtev_watcher_t *) request_memory;
  watcher->data = request_memory + aligned_request_size;
  watcher->close_cb = js_rtev_watcher_close_processor;

  RTNODE_RTEV_WATCHER_DATA(watcher)->jobject = jobject;
  RTNODE_RTEV_WATCHER_DATA(watcher)->close_cb = close_cb;
  jerry_acquire_value(jobject);

  jerry_set_object_native_pointer(jobject, watcher, native);
}

int rtnode_rtev_watcher_close(rtev_watcher_t *watcher) {
  if (watcher->state == RTEV_STATE_CLOSING ||
    watcher->state == RTEV_STATE_CLOSED) {
    return 0;
  }
  return rtev_watcher_close(watcher);
}
