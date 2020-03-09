
process.env.ELECTRON_RUN_AS_NODE = true;

const currentPath = window.location.hash;
if(currentPath.indexOf('/kungfu-code-editor') !== -1) {
    process.env.RENDERER_TYPE = 'codeEditor'
} else {
    process.env.RENDERER_TYPE = 'app'
}

console.log(process.env.RENDERER_TYPE)

// debug export
const { watcher, longfist } = require('__gUtils/kungfuUtils')
window.watcher = watcher;
window.longfist = longfist;

