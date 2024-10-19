'use strict';

const net = require('node:net');
const readline = require('node:readline');
const { stdin, stdout } = require('node:process');

const terminal = readline.createInterface({ input: stdin, output: stdout });

const PROMPT_STRING = '> ';

const prompt = () => void stdout.write(PROMPT_STRING);

const clearCurrentLine = () => {
  process.stdout.write('\x1b[2K\x1b[0G');
};

const main = () => {
  const options = { host: '127.0.0.1', port: 8000 };
  const socket = net.createConnection(options, () => {
    prompt();
    terminal.on('line', (message) => {
      socket.write(message);
      prompt();
    });
  });
  socket.on('data', (message) => {
    clearCurrentLine();
    console.log(message);
    stdin.write('> ' + terminal.line);
  });
  socket.setEncoding('utf-8');
};

main();
