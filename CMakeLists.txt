cmake_minimum_required(VERSION 3.5)

set(JERRY_GLOBAL_HEAP_SIZE "(128)")

file(GLOB JS_FILES src/js/*.js)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/rtnode-snapshots.c ${CMAKE_SOURCE_DIR}/rtnode-snapshots.h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND bash ${CMAKE_SOURCE_DIR}/tools/js2c.sh
  DEPENDS ${JS_FILES} ${CMAKE_SOURCE_DIR}/tools/js2c.sh ${CMAKE_SOURCE_DIR}/tools/js2c.py)

set(INCLUDE_LIST
  ${CMAKE_SOURCE_DIR}/deps/jerryscript/jerry-core/include
  ${CMAKE_SOURCE_DIR}/deps/jerryscript/jerry-ext/include
  ${CMAKE_SOURCE_DIR}/deps/jerryscript/jerry-port/default/include
  ${CMAKE_SOURCE_DIR}/deps/rtev/include
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/internal
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/modules
)
set(SRC_LIST
  ${CMAKE_SOURCE_DIR}/src/rtnode.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-binding.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-error.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-logger.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-modules.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-native.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-snapshots.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-common.c
  ${CMAKE_SOURCE_DIR}/src/rtnode-rtev-watcher.c
  ${CMAKE_SOURCE_DIR}/src/modules/rtnode-module-console.c
  ${CMAKE_SOURCE_DIR}/src/modules/rtnode-module-process.c
  ${CMAKE_SOURCE_DIR}/src/modules/rtnode-module-require.c
  ${CMAKE_SOURCE_DIR}/src/modules/rtnode-module-timer.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_async.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_env.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_function.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_lifetime.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_module.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_object_wrap.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_property.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_tsfn.c
  ${CMAKE_SOURCE_DIR}/src/napi/node_api_value.c
  )

if("${PLATFORM}" STREQUAL "espidf")
  set(EXTRA_COMPONENT_DIRS platforms/esp-idf)
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  project(rt-node)
elseif("${PLATFORM}" STREQUAL "unix")
  project(rt-node, C)
  include(platforms/unix/unix.cmake)
else()
  message(FATAL_ERROR "unsupport platform")
endif()
