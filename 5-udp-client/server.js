'use strict';

const dgram = require('node:dgram');

const socket = dgram.createSocket('udp4');

socket.on('message', (buffer, rinfo) => {
  console.log({ message: buffer.toString(), rinfo });
}).bind(8000, '127.0.0.1');
