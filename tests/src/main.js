
const pm2 = require('pm2-promise');
const fs = require('fs');
const path = require('path');
const fkill = require('fkill');
const os = require('os');
const osHomedir = require('os-homedir');

const starterPromise = Promise.resolve(null);

function getKungfuPath() {
    if (process.env.NODE_ENV === 'development') {
        return path.join(__dirname, '..', '..', 'core', 'build', 'kfc');
    }
    if (os.platform() === 'darwin') {
        return path.join('/Applications/Kungfu.app/Contents/Resources/kfc');
    }
}

function getKungfuHome() {
    if(os.platform() === 'darwin') {
        return path.join(osHomedir(), 'Library', 'Application\ Support', 'kungfu', 'app');
    }
}

function getTestCasePath() {
   if (process.env.NODE_ENV === 'development') {
       return path.join(__dirname, '..', 'cases');
   }
   if (os.platform() === 'darwin') {
       return path.join('/Applications/Kungfu.app/Contents/Resources/kfc');
   }
}

function wait(milliseconds) {
  return new Promise(resolve => setTimeout(resolve, milliseconds));
}


async function walk(dir) {
    let files = await fs.promises.readdir(dir);
    files = await Promise.all(files.map(async file => {
        const filePath = path.join(dir, file);
        const stats = await fs.promises.stat(filePath);
        if (stats.isDirectory()) return walk(filePath);
        else if(stats.isFile()) return filePath;
    }));

    return files.reduce((all, folderContents) => all.concat(folderContents), []);
}

function initializer() {
    const masterOptions = {
        'name': 'master',
        "script": 'kfc',
        'args': 'master',
        "cwd":  getKungfuPath(),
        "logType": "json",
        "mergeLogs": true,
        "logDateFormat": "YYYY-MM-DD HH:mm:ss",
        "autorestart": false,
        "maxRestarts": 1,
        "watch": false,
        "force": true,
        "execMode": "fork",
        "killTimeout": 16000
    };

    return pm2.start(masterOptions).then((apps) => {
        console.log('master started');
        return wait(1000);
    }).then(() => {
        const ledgerOptions = {
            'name': 'ledger',
            "script": 'kfc',
            'args': 'master',
            "cwd":  getKungfuPath(),
            "logType": "json",
            "mergeLogs": true,
            "logDateFormat": "YYYY-MM-DD HH:mm:ss",
            "autorestart": false,
            "maxRestarts": 1,
            "watch": false,
            "force": true,
            "execMode": "fork",
            "killTimeout": 16000
        }
        return pm2.start(ledgerOptions);
    }).then(() => {
        console.log('ledger started');
        return wait(1000);
    });
}

function clearUp() {
    return fkill('kfc', {'silent': true}).then(() => {
        console.log('kfc killed');
        return wait(1000)
    }).then(() => {
            return pm2.killDaemon();
    }).then(() => {
        console.log('pm2 daemon killed');
        return wait(1000);
    })
}

function execScript(filePath) {
    console.log('execute script: ', filePath)
    const fileContent = fs.readFileSync(filePath, 'utf-8');
    eval(fileContent);
    return Promise.resolve(null);
}

async function main() {
    const files = await walk(getTestCasePath());
    console.log(files);
    files.reduce((p, filePath) => p.then(() => initializer().then(() => {
        return execScript(filePath);
    }).then(() => {return clearUp()}).then(() => {console.log('clear up')})), starterPromise).then(() => {
        console.log('finished');
        process.exit();
    });
}

main()