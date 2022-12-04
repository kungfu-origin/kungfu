// SPDX-License-Identifier: Apache-2.0

const path = require('path');

function resolve(name) {
  const kfcDir = path.resolve(__dirname, '..', 'dist', 'kfc');
  const bin = process.platform === 'win32' ? `${name}.exe` : name;
  return path.resolve(kfcDir, bin);
}

module.exports = {
  kfc: resolve('kfc'),
  kfs: resolve('kfs'),
};
