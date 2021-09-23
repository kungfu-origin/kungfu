
process.env.RENDERER_TYPE = 'admin';



// debug export
const { kungfu, longfist, kungfuConfigStore, history } = require('@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils')
const { watcher } = require('@kungfu-trader/kungfu-uicc/io/kungfu/watcher');

window.watcher = watcher;
window.longfist = longfist;
window.kungfu = kungfu;
window.kungfuConfigStore = kungfuConfigStore;
window.kungfuHistory = history;


