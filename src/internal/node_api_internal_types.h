
/* Copyright 2018-present Rokid Co., Ltd. and other contributors
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

#ifndef JS_NODE_API_TYPES_H
#define JS_NODE_API_TYPES_H

#include "jerryscript.h"
//#include <uv.h>
#include "node_api.h"

typedef void (*rtnode_cleanup_hook_fn)(void* arg);

typedef struct rtnode_async_context_s rtnode_async_context_t;
typedef struct rtnode_async_work_s rtnode_async_work_t;
typedef struct rtnode_buffer_external_info_s rtnode_buffer_external_info_t;
typedef struct rtnode_callback_info_s rtnode_callback_info_t;
typedef struct rtnode_cleanup_hook_s rtnode_cleanup_hook_t;
typedef struct rtnode_function_info_s rtnode_function_info_t;
typedef struct rtnode_napi_env_s rtnode_napi_env_t;
typedef struct rtnode_object_info_s rtnode_object_info_t;
typedef struct rtnode_reference_s rtnode_reference_t;

typedef enum {
  napi_module_load_ok = 0,

  napi_module_no_pending,
  napi_module_no_nm_register_func,
} napi_module_load_status;

struct rtnode_cleanup_hook_s {
  rtnode_cleanup_hook_fn fn;
  void* arg;
  rtnode_cleanup_hook_t* next;
};

struct rtnode_buffer_external_info_s {
  napi_env env;
  void* external_data;
  void* finalize_hint;
  napi_finalize finalize_cb;
};

struct rtnode_reference_s {
  jerry_value_t jval;
  uint32_t refcount;

  rtnode_reference_t* prev;
  rtnode_reference_t* next;
};

#define JS_OBJECT_INFO_FIELDS \
  napi_env env;                  \
  void* native_object;           \
  napi_finalize finalize_cb;     \
  void* finalize_hint;           \
  rtnode_reference_t* ref_start;  \
  rtnode_reference_t* ref_end;

struct rtnode_object_info_s {
  JS_OBJECT_INFO_FIELDS
};

struct rtnode_function_info_s {
  JS_OBJECT_INFO_FIELDS

  napi_callback cb;
  void* data;
};

struct rtnode_callback_info_s {
  size_t argc;
  jerry_value_t* argv;
  jerry_value_t jval_this;
  jerry_value_t jval_func;

  jerryx_handle_scope handle_scope;
  rtnode_function_info_t* function_info;
};

struct rtnode_napi_env_s {
  napi_value pending_exception;
  napi_value pending_fatal_exception;
  napi_extended_error_info extended_error_info;

  /** Common function context */
  rtnode_callback_info_t* current_callback_info;
  pthread_t main_thread;

  rtnode_cleanup_hook_t* cleanup_hook;
};

struct rtnode_async_work_s {
  rtev_async_t work_req;

  napi_env env;
  napi_value async_resource;
  napi_value async_resource_name;
  napi_async_execute_callback execute;
  napi_async_complete_callback complete;
  void* data;
};

struct rtnode_async_context_s {
  napi_env env;
  napi_value async_resource;
  napi_value async_resource_name;
};

#endif // JS_NODE_API_TYPES_H
