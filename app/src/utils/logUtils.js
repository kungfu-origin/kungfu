import  { LOG_DIR } from '__gConfig/pathConfig';
const path = require('path')
const log4js = require('log4js');
log4js.configure({
    appenders: {
        app: {
            type: 'file', 
            filename: path.join(LOG_DIR, 'app.log')
        }
    },
    categories: { default: { appenders: ['app'], level: 'info' } }

})

export const logger = log4js.getLogger('app');
