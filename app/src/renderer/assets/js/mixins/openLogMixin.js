
import path from 'path';
import moment from 'moment';
import { LOG_DIR } from '__gConfig/pathConfig';


export default {
    
    methods: {
        handleOpenLogFile (id, allPath = false) {
            const tmk = moment().format('YYYYMMDD')
            let logPath = ''
            if (allPath) {
                logPath = id;
            } else {
                logPath = path.join(LOG_DIR, `${id}-${tmk}.log`);
            }

            this.$showLog(logPath)
        },
    }
}