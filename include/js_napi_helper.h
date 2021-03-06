#ifndef JS_NAPI_HELPER_H
#define JS_NAPI_HELPER_H

// Empty value so that macros here are able to return NULL or void
#define JS_NAPI_RETVAL_NOTHING // Intentionally blank #define

#define JS_NAPI_GET_AND_THROW_LAST_ERROR(env)                       \
  do {                                                              \
    const napi_extended_error_info* error_info;                     \
    napi_get_last_error_info((env), &error_info);                   \
    bool is_pending;                                                \
    napi_is_exception_pending((env), &is_pending);                  \
    /* If an exception is already pending, don't rethrow it */      \
    if (!is_pending) {                                              \
      const char* error_message = error_info->error_message != NULL \
                                    ? error_info->error_message     \
                                    : "empty error message";        \
      napi_throw_error((env), NULL, error_message);                 \
    }                                                               \
  } while (0)

#define JS_NAPI_ASSERT_BASE(env, assertion, message, ret_val)          \
  do {                                                                 \
    if (!(assertion)) {                                                \
      napi_throw_error((env), NULL,                                    \
                       "assertion (" #assertion ") failed: " message); \
      return ret_val;                                                  \
    }                                                                  \
  } while (0)

// Returns NULL on failed assertion.
// This is meant to be used inside napi_callback methods.
#define JS_NAPI_ASSERT(env, assertion, message) \
  JS_NAPI_ASSERT_BASE(env, assertion, message, NULL)

// Returns empty on failed assertion.
// This is meant to be used inside functions with void return type.
#define JS_NAPI_ASSERT_RETURN_VOID(env, assertion, message) \
  NAPI_ASSERT_BASE(env, assertion, message, JS_NAPI_RETVAL_NOTHING)

#define JS_NAPI_CALL_BASE(env, the_call, ret_val) \
  do {                                            \
    if ((the_call) != napi_ok) {                  \
      JS_NAPI_GET_AND_THROW_LAST_ERROR((env));    \
      return ret_val;                             \
    }                                             \
  } while (0)

// Returns NULL if the_call doesn't return napi_ok.
#define JS_NAPI_CALL(env, the_call) JS_NAPI_CALL_BASE(env, the_call, NULL)

// Returns empty if the_call doesn't return napi_ok.
#define JS_NAPI_CALL_RETURN_VOID(env, the_call) \
  JS_NAPI_CALL_BASE(env, the_call, JS_NAPI_RETVAL_NOTHING)

#define JS_DECLARE_NAPI_PROPERTY(name, func) \
  { (name), 0, (func), 0, 0, 0, napi_default, 0 }

#define JS_DECLARE_NAPI_GETTER(name, func) \
  { (name), 0, 0, (func), 0, 0, napi_default, 0 }

#define JS_NAPI_SET_NAMED_METHOD(env, target, prop_name, handler)    \
  do {                                                               \
    napi_status status;                                              \
    napi_value fn;                                                   \
    status = napi_create_function(env, NULL, 0, handler, NULL, &fn); \
    if (status != napi_ok)                                           \
      return NULL;                                                   \
                                                                     \
    status = napi_set_named_property(env, target, prop_name, fn);    \
    if (status != napi_ok)                                           \
      return NULL;                                                   \
  } while (0);

#endif // JS_NAPI_HELPER_H
