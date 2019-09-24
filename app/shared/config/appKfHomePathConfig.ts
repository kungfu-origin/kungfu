
const path = require('path')

const mainProcess = require('electron').app;
const renderProcess = (require('electron').remote || {}).app;

if(mainProcess) {
    const appName = 'kungfu'
    mainProcess.setName('kungfu')
    const appData = mainProcess.getPath('appData');
    mainProcess.setPath('userData', path.join(appData, appName));    
}

//ELEC_BASE
const getHomePath = () => {
    if (process.env.APP_TYPE === 'test') {
        return process.env.KF_HOME_BASE_DIR || ''; 
    } else {
        if (mainProcess) {
            return mainProcess.getPath('userData') 
        } else {
            return renderProcess.getPath('userData');
        }
    }
}

export const KF_HOME_BASE_DIR_RESOLVE = getHomePath();