const os = require('os');
const path = require('path');

const getHomePath = () => {
    switch (os.platform()) {
        case 'darwin':
            return path.join(
                os.homedir(),
                'Library',
                'Application Support',
                'kungfu',
            );
        case 'win32':
            return path.join(os.homedir(), 'AppData', 'Roaming', 'kungfu');
        case 'linux':
            return path.join(os.homedir(), '.config', 'kungfu');
    }
};

if (process.env.NODE_ENV !== 'development' && process.env.APP_TYPE === 'main') {
    global.__resources = path
        .join(__dirname, '/resources')
        .replace(/\\/g, '\\\\');
}

if (process.env.APP_TYPE === 'cli') {
    global.__kfResourcesPath = path
        .join(path.dirname(process.execPath), '..')
        .replace(/\\/g, '\\\\');
}

if (process.env.APP_TYPE === 'daemon') {
    global.__kfResourcesPath = path
        .join(__dirname, '..', '..', '..')
        .replace(/\\/g, '\\\\');
}

if (process.env.APP_TYPE === 'main' || process.env.APP_TYPE === 'renderer') {
    global.__kfResourcesPath = process.resourcesPath;
}

export const KF_HOME_BASE_DIR_RESOLVE: string = getHomePath();
