cmake_minimum_required(VERSION 3.5)
project(rtev)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

include_directories(./include)
include_directories(./src)
aux_source_directory(./src SRC_LIST)
add_library(rtev ${SRC_LIST} src/atomic-ops.h)

if("${RTEV_SAMPLE}" STREQUAL "ON")
  add_executable(sample sample/sample.c)
  target_link_libraries(sample rtev)
endif()
