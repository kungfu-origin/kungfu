import { platform } from '@kungfu-trader/kungfu-uicc/config/platformConfig'
const os = require('os');
const path = require('path');

const getHomePath = () => {
    switch (platform) {
        case 'mac':
            return path.join(os.homedir(), 'Library', 'Application\ Support', 'kungfu')
        case 'win':
            return path.join(os.homedir(), 'AppData', 'Roaming', 'kungfu')
        case 'linux':
            return path.join(os.homedir(), '.config', 'kungfu')
    }
}

if (process.env.NODE_ENV !== 'development' && process.env.APP_TYPE === 'main') {
    global.__resources = path.join(__dirname, '/resources').replace(/\\/g, '\\\\')
}

if (process.env.APP_TYPE === 'cli') {
    //@ts-ignore
    process.resourcesPath =  path.join(path.dirname(process.execPath), '..').replace(/\\/g, '\\\\')
}

if (process.env.APP_TYPE === 'daemon') {
    //@ts-ignore
    process.resourcesPath =  path.join(__dirname, '..', '..', '..').replace(/\\/g, '\\\\')
}



export const KF_HOME_BASE_DIR_RESOLVE: string = getHomePath()