
process.env.ELECTRON_RUN_AS_NODE = true;

process.env.RENDERER_TYPE = 'app'

// debug export
const { kungfu, longfist, kungfuConfigStore } = require('__gUtils/kungfuUtils')
const { watcher } = require('__io/kungfu/watcher');

window.watcher = watcher;
window.longfist = longfist;
window.kungfu = kungfu;
window.kungfuConfigStore = kungfuConfigStore;


