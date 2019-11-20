#include "js-error.h"
#include "js-common.h"

#define SYNTAX_ERROR_CONTEXT_SIZE 2

void js_on_fatal_error(jerry_type_t jerror, const jerry_char_t* source) {
  js_print_error(jerror, source);
}

void js_print_error(jerry_value_t jerror, const jerry_char_t* source) {
  jerror = jerry_get_value_from_error(jerror, true);
  JS_ASSERT(!jerry_value_is_error(jerror));
  const jerry_char_t* buffer = source;

  jerry_char_t err_str_buf[256];

  if (jerry_value_is_object(jerror)) {
    jerry_value_t stack_str = jerry_create_string((const jerry_char_t*)"stack");
    jerry_value_t backtrace_val = jerry_get_property(jerror, stack_str);
    jerry_release_value(stack_str);

    if (!jerry_value_is_error(backtrace_val) &&
        jerry_value_is_array(backtrace_val)) {
      JS_LOG_E("Exception backtrace:");

      uint32_t length = jerry_get_array_length(backtrace_val);

      /* This length should be enough. */
      if (length > 32) {
        length = 32;
      }

      for (uint32_t i = 0; i < length; i++) {
        jerry_value_t item_val = jerry_get_property_by_index(backtrace_val, i);

        if (!jerry_value_is_error(item_val) &&
            jerry_value_is_string(item_val)) {
          jerry_size_t str_size = jerry_get_utf8_string_size(item_val);

          if (str_size >= 256) {
            JS_LOG_E("%3u: [Backtrace string too long]", i);
          } else {
            jerry_size_t string_end =
              jerry_string_to_utf8_char_buffer(item_val, err_str_buf, str_size);
            assert(string_end == str_size);
            err_str_buf[string_end] = 0;

            JS_LOG_E("%3u: %s", i, err_str_buf);
          }
        }

        jerry_release_value(item_val);
      }
    }
    jerry_release_value(backtrace_val);
  }

  jerry_value_t err_str_val = jerry_value_to_string(jerror);
  jerry_size_t err_str_size = jerry_get_utf8_string_size(err_str_val);

  if (err_str_size >= 256) {
    const char msg[] = "[Error message too long]";
    err_str_size = sizeof(msg) / sizeof(char) - 1;
    memcpy(err_str_buf, msg, err_str_size + 1);
  } else {
    jerry_size_t string_end =
      jerry_string_to_utf8_char_buffer(err_str_val, err_str_buf, err_str_size);
    assert(string_end == err_str_size);
    err_str_buf[string_end] = 0;

    if (jerry_is_feature_enabled(JERRY_FEATURE_ERROR_MESSAGES) &&
        jerry_get_error_type(jerror) == JERRY_ERROR_SYNTAX) {
      jerry_char_t* string_end_p = err_str_buf + string_end;
      unsigned int err_line = 0;
      unsigned int err_col = 0;
      char* path_str_p = NULL;
      char* path_str_end_p = NULL;

      /* 1. parse column and line information */
      for (jerry_char_t* current_p = err_str_buf; current_p < string_end_p;
           current_p++) {
        if (*current_p == '[') {
          current_p++;

          if (*current_p == '<') {
            break;
          }

          path_str_p = (char*)current_p;
          JS_UNUSED(path_str_p);
          while (current_p < string_end_p && *current_p != ':') {
            current_p++;
          }

          path_str_end_p = (char*)current_p++;

          err_line =
            (unsigned int)strtol((char*)current_p, (char**)&current_p, 10);

          current_p++;

          err_col = (unsigned int)strtol((char*)current_p, NULL, 10);
          break;
        }
      } /* for */

      if (err_line != 0 && err_col != 0 && buffer != NULL) {
        unsigned int curr_line = 1;

        bool is_printing_context = false;
        unsigned int pos = 0;

        size_t source_size = strlen((const char*)buffer);

        /* Temporarily modify the error message, so we can use the path. */
        *path_str_end_p = '\0';

        /* Revert the error message. */
        *path_str_end_p = ':';

        /* 2. seek and print */
        while ((pos < source_size) && (buffer[pos] != '\0')) {
          if (buffer[pos] == '\n') {
            curr_line++;
          }

          if (err_line < SYNTAX_ERROR_CONTEXT_SIZE ||
              (err_line >= curr_line &&
               (err_line - curr_line) <= SYNTAX_ERROR_CONTEXT_SIZE)) {
            /* context must be printed */
            is_printing_context = true;
          }

          if (curr_line > err_line) {
            break;
          }

          if (is_printing_context) {
            fprintf(stderr, "%c", buffer[pos]);
          }

          pos++;
        }

        while (--err_col) {
          fprintf(stderr, "~");
        }

        fprintf(stderr, "^");
      }
    }
  }

  JS_LOG_E("Script Error: %s", err_str_buf);
  jerry_release_value(err_str_val);
}
