# cmake_minimum_required(VERSION 3.5)

# include(platforms/esp-idf.cmake)
# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(JERRYSCRIPT_SOURCE "${CMAKE_SOURCE_DIR}/deps/jerryscript")
set(RTEV_SOURCE "${CMAKE_SOURCE_DIR}/deps/rtev")

# Jerryscript setting here
set(JERRY_GLOBAL_HEAP_SIZE "(256)")

if("${PLATFORM}" STREQUAL "freertos")
  set(EXTRA_COMPONENT_DIRS platforms/esp-idf)
  include($ENV{IDF_PATH}/tools/cmake/project.cmake)
  project(rt-node)
elseif("${PLATFORM}" STREQUAL "unix")
  project(rt-node, C)
  include(platforms/unix/unix.cmake)
else()
  message(FATAL_ERROR "unsupport platform")
endif()
