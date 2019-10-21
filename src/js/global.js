'use strict';
var global = this

global.console = require('console')

global.process = require('process')

var timers = require('timers')
global.setTimeout = timers.setTimeout
global.setInterval = timers.setInterval
global.setImmediate = timers.setImmediate
global.clearTimeout = timers.clearTimeout
global.clearInterval = timers.clearInterval

// run user code
require('app')
