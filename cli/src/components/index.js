import strategy from './strategy/index'
import account from './account/index'
import monitor from './monitor/index'

const inquirer = require('inquirer');

export const monitPrompt = async () => {
    const { monit } = await inquirer.prompt([
        {
            type: 'list',
            name: 'monit',
            message: 'Select a type for monitor ',
            choices: ['all', 'account', 'strategy']
        }
    ])
    if(monit === 'strategy') return strategy();
    if(monit === 'account') return account();
    if(monit === 'all') return monitor();
}