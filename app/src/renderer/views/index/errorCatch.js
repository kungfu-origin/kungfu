import { remote } from 'electron'
const { dialog } = remote;


if (process.env.NODE_ENV === 'production') {
    process
    .on('unhandledRejection', (reason, p) => {
        console.error(reason, 'Unhandled Rejection at Promise', p);
    })
    .on('uncaughtException', (err) => {
        console.error(err, 'Uncaught Exception thrown');

        if (!window.AFTER_APP_MOUNTED) {
            dialog.showErrorBox('错误', err, a, b)
        } 
    });
}

