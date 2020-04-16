const kungfu = require('./kungfu.js');
const windowsKill = process.platform === 'win32' ? require('./windows-kill/index') : () => {};
exports.kungfu = kungfu;
exports.windowsKill = windowsKill