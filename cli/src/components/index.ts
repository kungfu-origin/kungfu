// import strategy from './strategy/index'
import account from './account/index'
import monitor from './monitor/index';
import { getAccountsStrategys, accountStrategyListStringify } from '@/assets/scripts/actions';
const inquirer = require('inquirer');
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

export const monitPrompt = async (list: boolean) => {  
    if(!list) return monitor();
    
    const { accounts, strategys } = await getAccountsStrategys();
    const accountStrategyList = accountStrategyListStringify(accounts, strategys);
    let answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'processName',
            message: 'Select target account / strategy process to monit    ',
            source: async (answersSoFar: any, input = '') => {
                return accountStrategyList
                    .filter((s: string) => s.indexOf(input) !== -1)
            }
        }
    ]);

    const { processName } = answers;
    const processId = processName.split(' ')[1].trim();
    const type = processName.split(' ')[0].indexOf('strategy') !== -1 ? 'strategys' : 'accounts';
    if (type === 'accounts') {
        return account(processId)
    }
    
    
}
