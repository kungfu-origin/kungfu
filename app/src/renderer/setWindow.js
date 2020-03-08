
process.env.ELECTRON_RUN_AS_NODE = true;

const currentPath = window.location.hash;
if(currentPath.indexOf('/code') !== -1) {
    process.env.RENDERER_TYPE = 'codeEditor'
}

// debug export
const { watcher, longfist } = require('__gUtils/kungfuUtils')
window.watcher = watcher;
window.longfist = longfist;
window.fileId = 0;
window.setTradingDay = (targetDay) => store.dispatch('setTradingDay', targetDay);
window.getTradingDay = () => store.state.BASE.tradingDay;

