cmake_minimum_required(VERSION 3.5)

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

set(COMPONENT_NAME rtnode)
set(COMPONENT_DIR deps/jerryscript)

include_directories(
  deps/jerryscript/jerry-core/include
  deps/jerryscript/jerry-ext/include
  deps/jerryscript/jerry-port/default/include
  deps/rtev/include
  deps/rtev/src
  include
  src/internal
  src
  src/modules
)
set(SRCS
  platforms/unix/main.c
  src/rtnode.c
  src/rtnode-binding.c
  src/rtnode-error.c
  src/rtnode-logger.c
  src/rtnode-modules.c
  src/rtnode-native.c
  src/rtnode-snapshots.c
  src/rtnode-common.c
  src/rtnode-rtev-watcher.c
  src/modules/rtnode-module-console.c
  src/modules/rtnode-module-process.c
  src/modules/rtnode-module-require.c
  src/modules/rtnode-module-timer.c
  src/napi/node_api.c
#  src/napi/node_api_async.c
  src/napi/node_api_env.c
  src/napi/node_api_function.c
  src/napi/node_api_lifetime.c
  src/napi/node_api_module.c
  src/napi/node_api_object_wrap.c
  src/napi/node_api_property.c
#  src/napi/node_api_tsfn.c
  src/napi/node_api_value.c
  deps/rtev/src/allocator.c
  deps/rtev/src/async.c
  deps/rtev/src/context.c
  deps/rtev/src/threadpool.c
  deps/rtev/src/tick.c
  deps/rtev/src/timer.c
  deps/rtev/src/watcher.c
)
add_executable(rtnode ${SRCS})

ExternalProject_Add(jerryscript
  PREFIX deps/jerryscript
  SOURCE_DIR ${JERRYSCRIPT_SOURCE}
  BUILD_IN_SOURCE 0
  BINARY_DIR jerryscript
  INSTALL_COMMAND "" # Do not install to host
  LIST_SEPARATOR | # Use the alternate list separator
  CMAKE_ARGS
    -DJERRY_GLOBAL_HEAP_SIZE=${JERRY_GLOBAL_HEAP_SIZE}
    -DJERRY_CMDLINE=OFF
    -DJERRY_ERROR_MESSAGES=ON
    -DJERRY_LINE_INFO=ON
    -DJERRY_EXTERNAL_CONTEXT=ON
    -DJERRY_SNAPSHOT_EXEC=ON
    -DJERRY_SNAPSHOT_SAVE=ON
    -DJERRY_PARSER=ON
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
)

set(JERRY_BUILD_PATH ${CMAKE_BINARY_DIR}/jerryscript)

target_link_libraries(${COMPONENT_NAME}
                      ${JERRY_BUILD_PATH}/lib/libjerry-core.a
                      ${JERRY_BUILD_PATH}/lib/libjerry-ext.a
                      ${JERRY_BUILD_PATH}/lib/libjerry-port-default-minimal.a)
