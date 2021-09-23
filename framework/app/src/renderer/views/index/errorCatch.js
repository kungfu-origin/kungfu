import { remote } from 'electron'
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';
const { dialog } = remote;


process
    .on('unhandledRejection', (reason, p) => {
        console.error(reason, 'Unhandled Rejection', p);
        logger.error(reason, 'Unhandled Rejection', p);
    })
    .on('uncaughtException', (err) => {
        if (!err) return;
        
        console.error('Uncaught Exception thrown', err);
        logger.error('Uncaught Exception thrown', err);

        if (!window.AFTER_APP_MOUNTED) {
            dialog.showErrorBox('错误', err.message)
        } 
    });
