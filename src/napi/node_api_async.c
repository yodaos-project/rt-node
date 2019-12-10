/* Copyright 2019-present Samsung Electronics Co., Ltd. and other contributors
 * Copyright 2018-present Rokid Co., Ltd. and other contributors
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

#include "internal/node_api_internal.h"

static void on_work(rv_worker_t* req) {
  js_async_work_t* async_work = (js_async_work_t*)req->data;
  if (async_work && async_work->execute != NULL) {
    async_work->execute(async_work->env, async_work->data);
  }
}

static void on_work_done(rv_worker_t* req) {
  js_async_work_t* async_work = (js_async_work_t*)req->data;
  JS_ASSERT(async_work != NULL);
  napi_status cb_status = napi_ok;

  napi_env env = async_work->env;
  napi_async_complete_callback complete = async_work->complete;
  void* data = async_work->data;

  if (complete != NULL) {
    jerryx_handle_scope scope;
    jerryx_open_handle_scope(&scope);
    /**
     * napi_async_work could be deleted by invocation of `napi_delete_asyncwork`
     * in its complete callback.
     */
    complete(env, cb_status, data);
    jerryx_close_handle_scope(scope);

    if (js_napi_is_exception_pending(env)) {
      jerry_value_t jval_err;
      jval_err = js_napi_env_get_and_clear_exception(env);
      if (jval_err == (uintptr_t)NULL) {
        jval_err = js_napi_env_get_and_clear_fatal_exception(env);
      }

      /** Argument cannot have error flag */
      js_on_fatal_error(jerry_get_value_from_error(jval_err, false), NULL);
      jerry_release_value(jval_err);
    }
  }

  // TODO
  //  js_run_next_tick();
}

static void on_work_close(rv_watcher_t* worker) {
  js_free(worker);
}

napi_status napi_create_async_work(napi_env env, napi_value async_resource,
                                   napi_value async_resource_name,
                                   napi_async_execute_callback execute,
                                   napi_async_complete_callback complete,
                                   void* data, napi_async_work* result) {
  NAPI_TRY_ENV(env);
  NAPI_WEAK_ASSERT(napi_invalid_arg, result != NULL);
  NAPI_WEAK_ASSERT(napi_invalid_arg, execute != NULL);
  NAPI_WEAK_ASSERT(napi_invalid_arg, complete != NULL);

  js_async_work_t* async_work = js_malloc(sizeof(js_async_work_t));
  rv_worker_t* work_req = &async_work->work_req;

  async_work->env = env;
  async_work->async_resource = async_resource;
  async_work->async_resource_name = async_resource_name;
  async_work->execute = execute;
  async_work->complete = complete;
  async_work->data = data;

  work_req->data = async_work;

  NAPI_ASSIGN(result, (napi_async_work)work_req);
  NAPI_RETURN(napi_ok);
}

napi_status napi_delete_async_work(napi_env env, napi_async_work work) {
  return napi_cancel_async_work(env, work);
}

napi_status napi_queue_async_work(napi_env env, napi_async_work work) {
  NAPI_TRY_ENV(env);
  rv_ctx_t* ctx = js_get_context()->rv;

  rv_worker_t* work_req = (rv_worker_t*)work;

  int status =
    rv_worker_start(ctx, work_req, on_work, on_work_done, on_work_close);
  if (status != 0) {
    const char* err_name = strerror(status);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, err_name);
  }
  NAPI_RETURN(napi_ok);
}

napi_status napi_cancel_async_work(napi_env env, napi_async_work work) {
  NAPI_TRY_ENV(env);
  rv_watcher_t* work_req = (rv_watcher_t*)work;
  int status = rv_watcher_close(work_req);
  if (status != 0) {
    const char* err_name = strerror(status);
    NAPI_RETURN_WITH_MSG(napi_generic_failure, err_name);
  }
  NAPI_RETURN(napi_ok);
}

napi_status napi_async_init(napi_env env, napi_value async_resource,
                            napi_value async_resource_name,
                            napi_async_context* result) {
  NAPI_TRY_ENV(env);

  js_async_context_t* ctx = js_malloc(sizeof(js_async_context_t));
  ctx->env = env;
  ctx->async_resource = async_resource;
  ctx->async_resource_name = async_resource_name;

  NAPI_ASSIGN(result, (napi_async_context)ctx);
  return napi_ok;
}

napi_status napi_async_destroy(napi_env env, napi_async_context async_context) {
  NAPI_TRY_ENV(env);

  js_async_context_t* ctx = (js_async_context_t*)async_context;
  js_free(ctx);

  return napi_ok;
}

napi_status napi_make_callback(napi_env env, napi_async_context async_context,
                               napi_value recv, napi_value func, size_t argc,
                               const napi_value* argv, napi_value* result) {
  NAPI_TRY_ENV(env);

  napi_status status = napi_call_function(env, recv, func, argc, argv, result);
  if (!js_napi_is_exception_pending(env)) {
    // TODO
    //    js_run_next_tick();
  } else {
    // In this case explicit napi_async_destroy calls won't be executed, so
    // free the context manually.
    js_free(async_context);
  }

  return status;
}
