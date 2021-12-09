//ELECTRON_RUN_AS_NODE 应用于通过process.execPath开启任务
process.env.ELECTRON_RUN_AS_NODE = 'true';
process.env.RENDERER_TYPE = 'app';
process.env.RELOAD_AFTER_CRASHED = process.argv.includes('reloadAfterCrashed')
    ? 'true'
    : 'false';
