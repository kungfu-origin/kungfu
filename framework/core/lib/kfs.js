#!/usr/bin/env node

const { kfs } = require('./executable');
const shell = require('./shell');

shell.run(kfs, process.argv.slice(2), true, {
  silent: true,
});
