import * as log4js from 'log4js';
import { buildProcessLogPath } from '@kungfu-trader/kungfu-uicc/config/pathConfig';

log4js.configure({
    appenders: {
        app: {
            type: 'file', 
            filename: buildProcessLogPath("app")
        }
    },
    categories: { default: { appenders: ['app'], level: 'info' } }
})

export const logger = log4js.getLogger('app');

