#include "js-uv-handle.h"

uv_handle_t* js_uv_handle_create(size_t handle_size,
                                 const jerry_value_t jobject,
                                 const jerry_object_native_info_t* native_info,
                                 size_t extra_data_size) {
  JS_ASSERT(jerry_value_is_object(jobject));

  /* Make sure that the jerry_value_t is aligned */
  size_t aligned_request_size = JS_ALIGNUP(handle_size, 8u);

  char* request_memory = js_malloc(
      aligned_request_size + sizeof(js_uv_handle_data) + extra_data_size);
  uv_handle_t* uv_handle = (uv_handle_t*)request_memory;
  uv_handle->data = request_memory + aligned_request_size;

  JS_UV_HANDLE_DATA(uv_handle)->jobject = jobject;
  JS_UV_HANDLE_DATA(uv_handle)->on_close_cb = NULL;
  jerry_acquire_value(jobject);

  jerry_set_object_native_pointer(jobject, uv_handle, native_info);

  return uv_handle;
}

static void js_uv_handle_close_processor(uv_handle_t* handle) {
  js_uv_handle_data* handle_data = JS_UV_HANDLE_DATA(handle);

  if (handle_data->on_close_cb != NULL) {
    handle_data->on_close_cb(handle);
  }

  // Decrease ref count of Javascript object. From now the object can be
  // reclaimed.
  jerry_release_value(handle_data->jobject);
  js_free(handle);
}

void js_uv_handle_close(uv_handle_t* handle, OnCloseHandler close_handler) {
  if (handle == NULL || uv_is_closing(handle)) {
    LOG_ERROR("Attempt to close uninitialized or already closed handle");
    return;
  }

  js_uv_handle_data* handle_data = JS_UV_HANDLE_DATA(handle);
  handle_data->on_close_cb = close_handler;
  uv_close(handle, js_uv_handle_close_processor);
}
