const os = require('os')
const childProcess = require('child_process');

switch (os.platform()) {
    case 'win32':
        return 'win';
    case 'darwin':
        childProcess.exec(`npm run build-mac`, (err, stdout) => {
            if(err) {
                console.error(err)
                process.exit(1)
            }
            console.log(stdout)
        })
        return 'mac';
    case 'linux':
        childProcess.exec(`npm run build-linux`, (err, stdout) => {
            if(err) {
                console.error(err)
                process.exit(1)
            }
            console.log(stdout)
        })
        return 'linux'
    default:
        return '';
};