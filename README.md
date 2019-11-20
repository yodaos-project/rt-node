# rt-node
[![License](https://img.shields.io/badge/licence-Apache%202.0-brightgreen.svg?style=flat)](LICENSE)

`rt-node` is a lightweight JavaScript framework for RTOS, also support unix like systems for debugging.

The following runtime modules are built in:
- timer
- require
- events
- utils
- console
- assert
- N-API

N-API is supported in order to be compatible with different embed JavaScript engines, the following N-API features are WIP:
- thread safe function

## Dependencies

- [JerryScript](./deps/jerryscript), a lightweight JavaScript engine
- [rv](./deps/rv), a tiny event loop library

## Example

JavaScript sources are packaged in `src/rtnode-snapshots.c/h`. Use `sh tools/js2c.sh` to package JavaScript Sources if the sources are changed.

`src/js/app.js` is the entry of JavaScript, here is an example:

```javascript
'use strict';
var EV = require('events').EventEmitter
var util = require('util')

function Timer() {
  EV.call(this)
}

util.inherits(Timer, EV)

Timer.prototype.run = function (timeout) {
  var self = this
  setTimeout(function () {
    self.emit('timeout', timeout)
  }, timeout)
}

var timer = new Timer()
timer.run(1000)
timer.on('timeout', function (timeout) {
  console.log('time out after ' + timeout + 'ms')
  console.log('memory usage: ', process.memoryUsage())
})

```

## Build

`rtnode` use [CMake](https://cmake.org) to build library or samples. The easiest way to build is as follows:

```shell
$ cmake -B./build -H.
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
  -DCMAKE_SYSTEM_NAME=Generic
$ make -C./build-xtensa -j8
```
 
## Sample

Currently support unix and esp-idf build framework.

For unix like systems
```shell
$ cmake -B./build -H. -DSAMPLE=unix
$ make -C./build
$ ./build/rtnode-unix/rtnode-unix # run sample
```

For esp-idf:
```shell
$ cmake -B./build-espidf -H. \
  -DCMAKE_C_COMPILER=xtensa-esp32-elf-gcc \
  -DCMAKE_SYSTEM_PROCESSOR=xtensa \
  -DCMAKE_SYSTEM_NAME=Generic \
  -DSAMPLE=esp-idf
$ make -C./build-espidf -j8
```

The esp-idf products will generate in `./build-espidf/rtnode-build`, then use `idf.py flash` to flash the binaries that you just built onto your ESP32 board. For more information, please refer to the [esp-idf document](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/#step-9-flash-onto-the-device).

## LICENSE

[Apache-2.0](./LICENSE.md)
