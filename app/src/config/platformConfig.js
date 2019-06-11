
export const platform = (() => {
    switch (process.platform) {
        case 'win32':
            return 'win';
        case 'win64':
            return 'win';
        case 'darwin':
            return 'mac';
    };
})()