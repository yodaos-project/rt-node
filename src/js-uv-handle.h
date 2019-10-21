#ifndef _JS_UV_HANDLE_
#define _JS_UV_HANDLE_

#include "js-common.h"

typedef void (*OnCloseHandler)(uv_handle_t*);

typedef struct {
  jerry_value_t jobject;
  OnCloseHandler on_close_cb;
} js_uv_handle_data;

#define JS_ALIGNUP(value, alignment) \
  (((value) + ((alignment)-1)) & ~((alignment)-1))

/**
 * Allocate and initialize an uv_handle_t structure with a jerry callback and
 * extra data.
 *
 * The allocated memory has the following layout:
 *
 *  |-------------|  <- start of uv_handle_t*
 *  | uv_handle_t |
 *  |             |
 *  |-------------|
 *  | PADDING     |  <- alignment padding
 *  |-------------|  <- start of the js_uv_handle_data struct
 *  | handle_data |
 *  |-------------|  <- start of the extra data if required
 *  |  extra      |
 *  |-------------|
 *
 */
uv_handle_t* js_uv_handle_create(size_t handle_size,
                                 const jerry_value_t jobject,
                                 const jerry_object_native_info_t* native_info,
                                 size_t extra_data_size);
void js_uv_handle_close(uv_handle_t* handle, OnCloseHandler close_handler);

/**
 * Returns a pointer to the handle data struct referenced
 * by the uv_handle_t->data member.
 */
#define JS_UV_HANDLE_DATA(UV_HANDLE) \
  ((js_uv_handle_data*)((UV_HANDLE)->data))

/**
 * Returns a char* pointer for any extra data.
 *
 * IMPORTANT!
 * Make sure that the extra data is correctly allocated by using the
 * js_uv_handle_create method call.
 */
#define JS_UV_HANDLE_EXTRA_DATA(UV_HANDLE) \
  ((char*)((char*)((UV_HANDLE)->data) + sizeof(js_uv_handle_data)))


#endif /* _JS_UV_HANDLE_ */
