
process.env.ELECTRON_RUN_AS_NODE = true;

const currentPath = window.location.hash;
if(currentPath.indexOf('/kungfuCodeEditor') !== -1) {
    process.env.RENDERER_TYPE = 'codeEditor'
} else {
    process.env.RENDERER_TYPE = 'app'
}

// debug export
const { watcher, longfist, kungfuConfigStore } = require('__gUtils/kungfuUtils')
window.watcher = watcher;
window.longfist = longfist;
window.kungfuConfigStore = kungfuConfigStore;

