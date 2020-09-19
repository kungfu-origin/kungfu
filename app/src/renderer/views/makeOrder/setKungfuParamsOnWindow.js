
process.env.ELECTRON_RUN_AS_NODE = true;

process.env.RENDERER_TYPE = 'makeOrder'

const { watcher } = require('__io/kungfu/watcher');

window.watcher = watcher;