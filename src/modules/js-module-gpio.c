#include "js-module-gpio.h"
#include "esp_err.h"
#include "driver/gpio.h"

JS_FUNCTION(mode_handler) {
  int pin_num = (int)js_object_to_number(jargv[0]);
  int pin_mode = (int)js_object_to_number(jargv[1]);
  esp_err_t status = gpio_set_direction(pin_num, pin_mode);
  if (status != ESP_OK) {
    LOG_ERROR("gpio_set_direction error!");
    return jerry_create_boolean(false);
  }
  return jerry_create_boolean(true);
}

JS_FUNCTION(write_handler) {
  int pin_num = (int)js_object_to_number(jargv[0]);
  uint32_t pin_level = (uint32_t)js_object_to_number(jargv[1]);
  esp_err_t status = gpio_set_level(pin_num, pin_level);
  if (status != ESP_OK) {
    LOG_ERROR("gpio_set_level error!\n");
    return jerry_create_boolean(false);
  }
  return jerry_create_boolean(true);
}

JS_FUNCTION(read_handler) {
  int pin_num = (int)js_object_to_number(jargv[0]);
  int level = gpio_get_level(pin_num);
  return jerry_create_number((double)level);
}

void js_init_gpio(jerry_value_t jexports,
  jerry_value_t jmodule, const char *filename) {
  js_object_set_method(jexports, "mode", mode_handler);
  js_object_set_method(jexports, "write", write_handler);
  js_object_set_method(jexports, "read", read_handler);
}
