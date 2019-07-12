
export const platform = ((): string => {
    switch (process.platform) {
        case 'win32':
            return 'win';
        case 'darwin':
            return 'mac';
        default:
            return '';
    };
})()