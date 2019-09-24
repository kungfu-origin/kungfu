import { platform } from '__gConfig/platformConfig'
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

export const KF_HOME_BASE_DIR_RESOLVE: string = getHomePath()