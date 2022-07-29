#!/usr/bin/env node

const { kfs } = require('./executable');
const shell = require('./shell');

shell.run(
  kfs,
  process.argv.slice(2),
  true,
  {
    silent: true,
  },
  {
    env: {
      KFS_DEV_PATH: Path('../../developer/sdk/src/kfs.js').resolve().as_posix(),
    },
  },
);
