
import path from 'path';
import { buildProcessLogPath } from '@kungfu-trader/kungfu-uicc/config/pathConfig';


export default {
    
    methods: {
        handleOpenLogFile (id, allPath = false) {
            let logPath = ''
            if (allPath) {
                logPath = id;
            } else {
                logPath = buildProcessLogPath(id)
            }

            this.$showLog(logPath)
        },
    }
}