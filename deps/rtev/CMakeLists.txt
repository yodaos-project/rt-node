cmake_minimum_required(VERSION 3.5)

include_directories(./include)
include_directories(./src)
aux_source_directory(./src RTEV_SRC)
add_library(rtev ${RTEV_SRC})

if("${RTEV_SAMPLE}" STREQUAL "ON")
  add_executable(sample sample/sample.c)
  target_link_libraries(sample rtev)
endif()
