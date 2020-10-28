
process.env.ELECTRON_RUN_AS_NODE = true;

process.env.RENDERER_TYPE = 'app'

// debug export
const { kungfu, longfist, kungfuConfigStore } = require('__gUtils/kungfuUtils')
const { watcher } = require('__io/kungfu/watcher');

// const { category, mode, InstrumentType, Side, Offset, ExchangeId, kfDebugger } = require('__gUtils/kungfuDebugger')

window.watcher = watcher;
window.longfist = longfist;
window.kungfu = kungfu;
window.kungfuConfigStore = kungfuConfigStore;

// window.kfDebugger = new kfDebugger
// window.category = category
// window.mode = mode
// window.InstrumentType = InstrumentType
// window.Side = Side
// window.Offset = Offset
// window.ExchangeId = ExchangeId

