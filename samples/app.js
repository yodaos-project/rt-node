'use strict';

// http depends on curl, remove it if there have not curl in target host
const http = require('http');

class HelloWorld {
  say() {
    console.log('hello world');
  }
}

const helloWorld = new HelloWorld();
setTimeout(() => {
  helloWorld.say();
}, 1000);

http.get('http://www.example.com', (body) => {
  console.log('get body:');
  console.log(body);
});

console.log('app start');
