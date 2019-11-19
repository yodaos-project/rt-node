set(INCLUDE_LIST
  ${CMAKE_SOURCE_DIR}/../../deps/jerryscript/jerry-core/include
  ${CMAKE_SOURCE_DIR}/../../deps/jerryscript/jerry-ext/include
  ${CMAKE_SOURCE_DIR}/../../deps/jerryscript/jerry-port/default/include
  ${CMAKE_SOURCE_DIR}/../../deps/rtev/include
  ${CMAKE_SOURCE_DIR}/../../include
  ${CMAKE_SOURCE_DIR}/../../src/internal
  ${CMAKE_SOURCE_DIR}/../../src
  ${CMAKE_SOURCE_DIR}/../../src/modules)
idf_component_register(SRCS main.c INCLUDE_DIRS ${INCLUDE_LIST})
