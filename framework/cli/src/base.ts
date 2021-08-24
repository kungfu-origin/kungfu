const colors = require('colors');

declare global {
    interface Console {
        success: Function
    }
}

console.success = (log: string) => {
    console.log(`${colors.green('SUCCESS')} ${log}`)
}

console.error = (log: string) => {
    console.log(`${colors.red('ERROR')} ${log}`)
}

export const { initConfig } = require('@kungfu-trader/kungfu-uicc/assets/base')
initConfig();
