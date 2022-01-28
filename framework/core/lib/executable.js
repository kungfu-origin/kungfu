const path = require('path');

const kfcDir = path.resolve(__dirname, '..', 'dist', 'kfc');

module.exports = path.resolve(kfcDir, process.platform === 'win32' ? 'kfc.exe' : 'kfc');
