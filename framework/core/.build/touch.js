const fs = require('fs');
const path = require('path');

const touchTime = new Date();

for (const arg of process.argv.slice(2)) {
  const file = path.resolve(arg)
  if (fs.existsSync(file)) {
    fs.utimesSync(file, fs.lstatSync(file).atime, touchTime)
  }
}
