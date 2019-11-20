#ifndef _JS_MODULES_H_
#define _JS_MODULES_H_

#include "js-common.h"
#include "js-native.h"
#include "js-snapshots.h"

void js_load_global_modules();

const js_snapshot_module_t* js_get_js_module(const char *name);

const js_native_module_t* js_get_native_module(const char *name,
                                                   enum js_native_module_type type);

#endif // _JS_MODULES_H_
