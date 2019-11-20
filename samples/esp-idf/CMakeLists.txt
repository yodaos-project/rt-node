cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(rtnode-esp-idf)

set(LINK_DIR ${CMAKE_BINARY_DIR}/..)
target_link_libraries(rtnode-esp-idf.elf
  ${LINK_DIR}/librtnode.a
  ${LINK_DIR}/deps/rv/librv.a
  ${LINK_DIR}/lib/libjerry-core.a
  ${LINK_DIR}/lib/libjerry-ext.a
  ${LINK_DIR}/lib/libjerry-port-default-minimal.a)
