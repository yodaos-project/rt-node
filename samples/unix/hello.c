#include "js.h"

static napi_value hello(napi_env env, napi_callback_info info) {
  const char* str = "hi rt-node";
  napi_value nstr;
  JS_NAPI_CALL(env, napi_create_string_utf8(env, str, NAPI_AUTO_LENGTH, &nstr));
  return nstr;
}

static napi_value init_hello(napi_env env, napi_value exports) {
  JS_NAPI_SET_NAMED_METHOD(env, exports, "getContent", hello);
  return exports;
}

NAPI_MODULE(hello, init_hello);
