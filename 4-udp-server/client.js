'use strict';

const dgram = require('node:dgram');

for (let i = 0; i < 10; i++) {
  const socket = dgram.createSocket('udp4');
  socket.send('Hello world ' + i, 8000, '127.0.0.1', (error, bytes) => {
    console.log({ error, bytes, index: i });
    socket.close();
  });
}