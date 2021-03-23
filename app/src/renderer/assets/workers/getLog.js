const { getLog, dealLogMessage } = require('__gUtils/busiUtils');

self.addEventListener('message', (e) => {
    const { logPath, searchKeyword } = e.data || {}

    console.log(logPath, searchKeyword, '--')

    getLog(logPath, searchKeyword, dealLogMessage).then(({ list }) => {
        self.postMessage({
            list,
            logPath,
            searchKeyword
        })
    })
})

