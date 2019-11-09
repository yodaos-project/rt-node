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
- async
- thread safe function

## Dependencies

- [JerryScript](./deps/jerryscript), a lightweight JavaScript engine
- [rtev](./deps/rtev), a tiny event loop library

## Example

JavaScript sources are packaged in `src/js-snapshots.c/h`. Use `sh tools/js2c.sh` to package JavaScript Sources if the sources are changed.

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

## Build & Install

- unix like

```shell
$ cmake -B./build -H. -DPLATFORM=unix     # build for unix platforms
$ make -C./build
$ ./build/rtnode
```

- freeRTOS based

Currently support esp-idf build framework.

```shell
$ cmake -B./build -H. -DPLATFORM=espidf
$ make -C./build
$ sh tools/esp_flash.sh -p $device_port
```

## LICENSE

[Apache-2.0](./LICENSE.md)
