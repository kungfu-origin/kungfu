const os = require('os')
export const platform = ((): string => {
    switch (os.platform()) {
        case 'win32':
            return 'win';
        case 'darwin':
            return 'mac';
        case 'linux':
            return 'linux'
        default:
            return '';
    };
})()