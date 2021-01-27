import { remote } from 'electron'
const { dialog } = remote;

process.on('uncaughtException', err => {
    if (!window.MAIN_RENDERED) {
        dialog.showErrorBox('错误', err.message)
        return;
    } 

    if (process.env.NODE_ENV !== 'production') {
        dialog.showErrorBox('错误', err.message)
    }

})