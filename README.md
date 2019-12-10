# rt-node
[![License](https://img.shields.io/badge/licence-Apache%202.0-brightgreen.svg?style=flat)](LICENSE)

`rt-node` is a lightweight JavaScript framework for RTOS, also support unix like systems for debugging.

The following runtime modules are built in:
- timer
- require
- utils
- console
- N-API

N-API is supported in order to be compatible with different embed JavaScript engines, the following N-API features are WIP:
- thread safe function

## Dependencies

- [JerryScript](./deps/jerryscript), a lightweight JavaScript engine
- [rv](./deps/rv), a tiny event loop library

## Example

JavaScript sources are packaged in `src/rtnode-snapshots.c/h`. Use `sh tools/js2c.sh` to package JavaScript sources if the sources are changed.

set `JS_ROOT` for `cmake` as your JavaScript sources root directory, here is an example:

```javascript

'use strict';

// http depends on curl, remove it if there have not curl in target host
const curl = require('curl');

console.log('app start');

class Speaker {
  constructor(content) {
    this.content = content;
  }

  say() {
    console.log(this.content);
  }
}

const speaker = new Speaker('hello world');
setTimeout(() => {
  speaker.say();
}, 3000);

const startTime = Date.now();
curl.get('http://www.example.com', (body) => {
  console.log(`get body in ${Date.now() - startTime}ms`);
  console.log(body.replace(/[ \n]/g, ''));
});

```

## Build

`rtnode` use [CMake](https://cmake.org) to build library or samples. The easiest way to build is as follows:

```shell
$ cmake -B./build -H. -DJS_ROOT=Your_js_files_root_directory
$ make -C./build -j8
```

The above commands will generate `librtnode.a` in `./build` directory.

For cross compile, add the following flags:
- CMAKE_C_COMPILER, full path for c compiler
- CMAKE_SYSTEM_PROCESSOR, the name of the CPU CMake is building for
- CMAKE_SYSTEM_NAME, set `Generic` to indicate cross compile

here is an example for `Xtensa` toolchain:

```shell
$ cmake -B./build-xtensa -H. \
  -DCMAKE_C_COMPILER=xtensa-esp32-elf-gcc \
  -DCMAKE_SYSTEM_PROCESSOR=xtensa \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DJS_ROOT=Your_js_files_root_directory
$ make -C./build-xtensa -j8
```
 
## Sample

Currently support unix and esp-idf build framework.

For unix like systems
```shell
$ cmake -B./build -H. -DSAMPLE=unix -DJS_ROOT=./samples -DJERRY_PROFILE=es2015-subset
$ make -C./build
$ ./build/rtnode-unix/rtnode-unix # run sample
```

For esp-idf:
```shell
$ cmake -B./build-espidf -H. \
  -DCMAKE_C_COMPILER=xtensa-esp32-elf-gcc \
  -DCMAKE_SYSTEM_PROCESSOR=xtensa \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DSAMPLE=esp-idf \
  -DJS_ROOT=./samples \
  -DJERRY_PROFILE=es2015-subset
$ make -C./build-espidf -j8
```

The esp-idf products will generate in `./build-espidf/rtnode-build`, then use `idf.py flash` to flash the binaries that you just built onto your ESP32 board. For more information, please refer to the [esp-idf document](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/#step-9-flash-onto-the-device).

## LICENSE

[Apache-2.0](./LICENSE)
