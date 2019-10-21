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

