#ifndef _RTNODE_MODULES_H_
#define _RTNODE_MODULES_H_

#include "rt-node.h"
#include "rtnode-native.h"
#include "rtnode-snapshots.h"

void rtnode_load_global_modules();

const rtnode_snapshot_module_t* rtnode_get_js_module(const char *name);

const rtnode_native_module_t* rtnode_get_native_module(const char *name,
                                                   enum rtnode_native_module_type type);

#endif // _RTNODE_MODULES_H_
