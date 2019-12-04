const path = require('path');
const colors = require('colors');

declare global {
    interface Console {
        success: Function
    }
}

export {}

console.success = (log: string) => {
    console.log(`${colors.green('SUCCESS')} ${log}`)
}

console.error = (log: string) => {
    console.log(`${colors.red('ERROR')} ${log}`)
}

if (process.env.NODE_ENV === 'production') {
    //@ts-ignore
    process.resourcesPath =  path.join(path.dirname(process.execPath), '..').replace(/\\/g, '\\\\')
}

const { initDB, initConfig } = require('__app/src/main/base')

initConfig();
initDB();