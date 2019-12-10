#include "common.h"
#include "curl/curl.h"

typedef struct {
  char* ptr;
  size_t len;
} string;

typedef struct {
  char* url;
  napi_ref cb;
  napi_async_work work;
  string body;
} req_t;

static void init_string(string* s) {
  s->len = 0;
  s->ptr = malloc(s->len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

static size_t write_func(void* ptr, size_t size, size_t nmemb, string* s) {
  size_t new_len = s->len + size * nmemb;
  s->ptr = realloc(s->ptr, new_len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr + s->len, ptr, size * nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size * nmemb;
}

static void execute(napi_env env, void* data) {
  req_t* req = (req_t*)data;
  CURL* curl = curl_easy_init();
  if (curl) {
    init_string(&req->body);
    curl_easy_setopt(curl, CURLOPT_URL, req->url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &req->body);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      JS_LOG_E("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);
  }
}

static void complete(napi_env env, napi_status status, void* data) {
  req_t* req = (req_t*)data;

  napi_value argv[1];
  NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, req->body.ptr,
                                                     req->body.len, argv));

  napi_value callback;
  NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, req->cb, &callback));
  napi_value global;
  NAPI_CALL_RETURN_VOID(env, napi_get_global(env, &global));

  napi_value result;
  NAPI_CALL_RETURN_VOID(env, napi_call_function(env, global, callback, 1, argv,
                                                &result));

  NAPI_CALL_RETURN_VOID(env, napi_delete_reference(env, req->cb));
  NAPI_CALL_RETURN_VOID(env, napi_delete_async_work(env, req->work));

  js_free(req->url);
  free(req->body.ptr);
  js_free(req);
}

static napi_value do_get(napi_env env, napi_callback_info info) {
  req_t* req = (req_t*)js_malloc(sizeof(req_t));
  size_t argc = 2;
  napi_value argv[argc];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, NULL, NULL));

  napi_valuetype t;
  NAPI_CALL(env, napi_typeof(env, argv[0], &t));
  NAPI_ASSERT(env, t == napi_string, "Wrong first argument, string expected.");
  NAPI_CALL(env, napi_typeof(env, argv[1], &t));
  NAPI_ASSERT(env, t == napi_function,
              "Wrong second argument, function expected.");

  size_t length;
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], NULL, 0, &length));
  req->url = (char*)js_malloc(length + 1);
  NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], req->url, length,
                                            &length));
  req->url[length] = '\0';

  NAPI_CALL(env, napi_create_reference(env, argv[1], 1, &req->cb));

  napi_value nurl;
  NAPI_CALL(env, napi_create_string_utf8(env, "url", NAPI_AUTO_LENGTH, &nurl));
  NAPI_CALL(env, napi_create_async_work(env, NULL, nurl, execute, complete, req,
                                        &req->work));
  NAPI_CALL(env, napi_queue_async_work(env, req->work));

  napi_value undefined_value;
  napi_get_undefined(env, &undefined_value);
  return undefined_value;
}

napi_value init_http(napi_env env, napi_value exports) {
  SET_NAMED_METHOD(env, exports, "get", do_get);
  return exports;
}

NAPI_MODULE(http, init_http);
