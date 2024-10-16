'use strict';

const net = require('node:net');

const options = { host: '127.0.0.1', port: 8000 };
const connection = net.createConnection(options, () => {
  connection.on('data', (data) => {
    console.log({ data: data.toString('utf-8') });
  });
});
