const { getLog, dealLogMessage } = require('__gUtils/busiUtils');

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

