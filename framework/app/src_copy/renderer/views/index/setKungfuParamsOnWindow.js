
process.env.ELECTRON_RUN_AS_NODE = true;
process.env.RENDERER_TYPE = 'app';



// debug export
const { kungfu, longfist, kungfuConfigStore, history } = require('@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils')
const { watcher, startGetKungfuWatcherStep } = require('@kungfu-trader/kungfu-uicc/io/kungfu/watcher');

window.watcher = watcher;
window.longfist = longfist;
window.kungfu = kungfu;
window.kungfuConfigStore = kungfuConfigStore;
window.kungfuHistory = history;

startGetKungfuWatcherStep();


