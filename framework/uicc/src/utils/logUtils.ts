import * as log4js from 'log4js';
import { LOG_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';

const path = require('path')

log4js.configure({
    appenders: {
        app: {
            type: 'file',
            filename: path.resolve(LOG_DIR, 'app.log')
        }
    },
    categories: { default: { appenders: ['app'], level: 'info' } }
})

export const logger = log4js.getLogger('app');

