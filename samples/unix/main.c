#include "js.h"

extern napi_module http;

int main(int argc, char** argv) {
  napi_module_register(&http);
  js_start();
  return 0;
}
