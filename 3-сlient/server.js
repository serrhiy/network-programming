'use strict';

const net = require('node:net');

net.createServer((socket) => {
  socket.end('Hello world!');
}).listen(8000, '127.0.0.1');
