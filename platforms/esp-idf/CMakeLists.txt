set(COMPONENT_ADD_INCLUDEDIRS
  ../../deps/jerryscript/jerry-core/include
  ../../deps/jerryscript/jerry-ext/include
  ../../deps/jerryscript/jerry-port/default/include
  ../../deps/rtev/include
  ../../deps/rtev/src
  ../../src
  ../../src/modules
  ../../include
)
set(COMPONENT_SRCS
  main.c
  ../../src/js-binding.c
  ../../src/js-error.c
  ../../src/js-logger.c
        ../../src/js-main.c
  ../../src/js-modules.c
  ../../src/js-native.c
  ../../src/js-snapshots.c
  ../../src/js-common.c
  ../../src/js-rtev-watcher.c
  ../../src/modules/js-module-console.c
  ../../src/modules/js-module-process.c
  ../../src/modules/js-module-require.c
  ../../src/modules/js-module-timer.c
  ../../deps/rtev/src/allocator.c
  ../../deps/rtev/src/async.c
  ../../deps/rtev/src/context.c
  ../../deps/rtev/src/threadpool.c
  ../../deps/rtev/src/tick.c
  ../../deps/rtev/src/timer.c
  ../../deps/rtev/src/watcher.c
)

register_component()

add_definitions(-D__FREERTOS__=1)

# Xtensa processor architecture optimization
set(EXTERNAL_COMPILE_FLAGS -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -mlongcalls -nostdlib -w)
string(REPLACE ";" "|" EXTERNAL_COMPILE_FLAGS_ALT_SEP "${EXTERNAL_COMPILE_FLAGS}")

externalproject_add(jerryscript_build
  PREFIX ${COMPONENT_DIR}
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
    -DENABLE_LTO=OFF # FIXME: This option must be turned off or the cross-compiler settings will be overwritten
    -DCMAKE_C_COMPILER_WORKS=true # cross-compiler
    -DCMAKE_SYSTEM_NAME=Generic
    -DCMAKE_SYSTEM_PROCESSOR=xtensa
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DEXTERNAL_COMPILE_FLAGS=${EXTERNAL_COMPILE_FLAGS_ALT_SEP}
    -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS}
    -DCMAKE_LINKER=${CMAKE_LINKER}
    -DCMAKE_AR=${CMAKE_AR}
    -DCMAKE_NM=${CMAKE_NM}
    -DCMAKE_RANLIB=${CMAKE_RANLIB}
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER
)
add_dependencies(${COMPONENT_NAME} jerryscript_build)

set(JERRY_BUILD_PATH ${CMAKE_BINARY_DIR}/${COMPONENT_NAME}/jerryscript)

target_link_libraries(${COMPONENT_NAME}
                      ${JERRY_BUILD_PATH}/lib/libjerry-core.a
                      ${JERRY_BUILD_PATH}/lib/libjerry-ext.a
                      ${JERRY_BUILD_PATH}/lib/libjerry-port-default-minimal.a)
