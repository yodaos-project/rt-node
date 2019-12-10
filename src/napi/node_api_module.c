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

#include "internal/node_api_internal.h"
#include "jerryscript-ext/handle-scope.h"
#include "js-common.h"

static QUEUE napi_modules;
static bool is_inited = false;

typedef struct {
  QUEUE node;
  napi_module* mod;
} napi_module_internal;

void napi_init_module_queue() {
  if (is_inited) {
    return;
  }
  QUEUE_INIT(&napi_modules);
  is_inited = true;
}

void napi_uninit_module_queue() {
  QUEUE* q;
  napi_module_internal* m;
  while (!QUEUE_EMPTY(&napi_modules)) {
    q = QUEUE_HEAD(&napi_modules);
    m = QUEUE_DATA(q, napi_module_internal, node);
    QUEUE_REMOVE(q);
    js_free(m);
  }
  is_inited = false;
}

void napi_module_register(napi_module* mod) {
  napi_init_module_queue();
  QUEUE* q;
  napi_module_internal* m;
  QUEUE_FOREACH(q, &napi_modules) {
    m = QUEUE_DATA(q, napi_module_internal, node);
    JS_ASSERT(strcmp(m->mod->nm_modname, mod->nm_modname) != 0);
  }
  napi_module_internal* internal_mod =
    (napi_module_internal*)js_malloc(sizeof(napi_module_internal));
  internal_mod->mod = mod;
  QUEUE_INIT(&internal_mod->node);
  QUEUE_INSERT_TAIL(&internal_mod->node, &napi_modules);
}

jerry_value_t napi_require_module(const char* name) {
  jerry_value_t jexports = jerry_create_undefined();
  QUEUE* q;
  napi_module_internal* mod = NULL;
  QUEUE_FOREACH(q, &napi_modules) {
    napi_module_internal* m;
    m = QUEUE_DATA(q, napi_module_internal, node);
    if (strcmp(m->mod->nm_modname, name) == 0) {
      mod = m;
      break;
    }
  }
  if (!mod) {
    goto end;
  }
  napi_addon_register_func init =
    (napi_addon_register_func)mod->mod->nm_register_func;

  if (init == NULL) {
    goto end;
  }

  napi_env env = js_get_current_napi_env();

  jerryx_handle_scope scope;
  jerryx_open_handle_scope(&scope);

  jerry_release_value(jexports);
  jexports = jerry_create_object();
  napi_value nvalue_ret = (*init)(env, AS_NAPI_VALUE(jexports));

  if (nvalue_ret == NULL) {
    jerry_release_value(jexports);
    jexports = jerry_create_undefined();
  } else {
    jerry_value_t jval_ret = AS_JERRY_VALUE(nvalue_ret);
    if (jval_ret != jexports) {
      jerry_release_value(jexports);
      jerryx_remove_handle(scope, jval_ret, &jval_ret);
    }
    jexports = jval_ret;
  }

  jerryx_close_handle_scope(scope);

  if (js_napi_is_exception_pending(env)) {
    jerry_value_t jval_err;
    jval_err = js_napi_env_get_and_clear_exception(env);
    JS_CHECK_FATAL_ERROR(jval_err, NULL);
  }

end:
  if (jerry_value_is_undefined(jexports)) {
    return jerry_create_undefined();
  } else {
    jerry_value_t jmodule = jerry_create_object();
    js_object_set_property(jmodule, "exports", jexports);
    jerry_release_value(jexports);
    return jmodule;
  }
}
