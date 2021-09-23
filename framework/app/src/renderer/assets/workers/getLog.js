const { getLog, dealLogMessage } = require('@kungfu-trader/kungfu-uicc/utils/busiUtils');

self.addEventListener('message', (e) => {
    const { logPath, searchKeyword } = e.data || {}

    getLog(logPath, searchKeyword, dealLogMessage).then(({ list }) => {
        self.postMessage({
            list,
            logPath,
            searchKeyword
        })
    })
})

