
process.env.ELECTRON_RUN_AS_NODE = true;
process.env.RENDERER_TYPE = 'app';



// debug export
const { kungfu, longfist, kungfuConfigStore, history } = require('__io/kungfu/kungfuUtils')
const { watcher, startGetKungfuWathcerStep } = require('__io/kungfu/watcher');

window.watcher = watcher;
window.longfist = longfist;
window.kungfu = kungfu;
window.kungfuConfigStore = kungfuConfigStore;
window.kungfuHistory = history;

startGetKungfuWathcerStep();


