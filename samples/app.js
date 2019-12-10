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
