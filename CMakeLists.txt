cmake_minimum_required(VERSION 2.8)
project(rtnode C)

# set jerry build options
set(ENABLE_LTO                      OFF     CACHE BOOL "Enable LTO")
set(JERRY_CMDLINE                   OFF     CACHE BOOL "Build jerry command line tool?")
set(JERRY_PORT_DEFAULT              ON      CACHE BOOL "Build default jerry port implementation?")
set(JERRY_EXT                       ON      CACHE BOOL "Build jerry-ext?")
set(JERRY_LIBM                      ON      CACHE BOOL "Build and use jerry-libm?")
set(JERRY_ERROR_MESSAGES            ON      CACHE BOOL   "Enable error messages?")
set(JERRY_EXTERNAL_CONTEXT          ON      CACHE BOOL   "Enable external context?")
set(JERRY_PARSER                    ON      CACHE BOOL   "Enable javascript-parser?")
set(JERRY_LINE_INFO                 ON      CACHE BOOL   "Enable line info?")
set(JERRY_MEM_STATS                 OFF     CACHE BOOL   "Enable memory statistics?")
set(JERRY_PROFILE                   "es5.1" CACHE STRING "Use default or other profile?")
set(JERRY_SNAPSHOT_EXEC             ON      CACHE BOOL   "Enable executing snapshot files?")
set(JERRY_GLOBAL_HEAP_SIZE          "(128)" CACHE STRING "Size of memory heap, in kilobytes")

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "xtensa")
  # FIXME add platform flag to distinguish between different platforms
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mlongcalls")
  add_definitions(-D__ESP_IDF__=1)
endif()

# package src/js/*.js to src/rtnode-snapshots.c before build
file(GLOB JS_FILES src/js/*.js)
add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/src/js-snapshots.c ${CMAKE_SOURCE_DIR}/js-snapshots.h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND bash ${CMAKE_SOURCE_DIR}/tools/js2c.sh
  DEPENDS ${JS_FILES} ${CMAKE_SOURCE_DIR}/tools/js2c.sh ${CMAKE_SOURCE_DIR}/tools/js2c.py)

# set header search path
set(INCLUDE_LIST
  ${CMAKE_SOURCE_DIR}/deps/jerryscript/jerry-core/include
  ${CMAKE_SOURCE_DIR}/deps/jerryscript/jerry-ext/include
  ${CMAKE_SOURCE_DIR}/deps/jerryscript/jerry-port/default/include
  ${CMAKE_SOURCE_DIR}/deps/rv/include
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/internal
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/modules)
include_directories(${INCLUDE_LIST})

# set source list
aux_source_directory(src/ SRC_LIST)
aux_source_directory(src/napi NAPI_LIST)
aux_source_directory(src/modules MODULES_LIST)
set(SNAPSHOT_SRC src/js-snapshots.c)

# build deps
add_subdirectory(deps/jerryscript)
add_subdirectory(deps/rv)

# build library
add_library(rtnode ${SRC_LIST} ${NAPI_LIST} ${MODULES_LIST} ${SNAPSHOT_SRC})
target_link_libraries(rtnode
  rv
  jerry-core
  jerry-ext
  jerry-port-default-minimal)

# build sample
if(DEFINED SAMPLE)
  if("${SAMPLE}" STREQUAL "unix")
    add_subdirectory(samples/unix)
  elseif("${SAMPLE}" STREQUAL "esp-idf")
    include(ExternalProject)
    externalproject_add(rtnode-esp-idf
      SOURCE_DIR ${CMAKE_SOURCE_DIR}/samples/esp-idf/
      BINARY_DIR ${CMAKE_BINARY_DIR}/rtnode-build
      CMAKE_ARGS)
  else()
    message(FATAL_ERROR "unsupport platform ${SAMPLE}")
  endif()
endif()
