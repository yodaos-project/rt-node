cmake_minimum_required(VERSION 3.5)

set(JERRY_GLOBAL_HEAP_SIZE ${JERRY_GLOBAL_HEAP_SIZE})
set(DJERRY_CMDLINE OFF)
set(JERRY_ERROR_MESSAGES ON)
set(JERRY_LINE_INFO ON)
set(JERRY_EXTERNAL_CONTEXT ON)
set(JERRY_SNAPSHOT_EXEC ON)
set(JERRY_SNAPSHOT_SAVE ON)
set(JERRY_PARSER ON)

include_directories(${INCLUDE_LIST})

add_subdirectory(deps/jerryscript jerryscript)
add_subdirectory(deps/rtev rtev)

add_executable(rtnode platforms/unix/main.c ${SRC_LIST})
target_link_libraries(rtnode
  rtev
  jerry-core
  jerry-ext
  jerry-port-default-minimal)
