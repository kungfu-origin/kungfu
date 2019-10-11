import tradingData from '@/components/tradingData/index'
import monitor from '@/components/monitor/index';
import { getAccountsStrategys, accountStrategyListStringify } from '@/assets/scripts/actions';
import { delaySeconds } from '__gUtils/busiUtils';
import { startLedger, startMaster } from '__gUtils/processUtils';
const inquirer = require('inquirer');
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

export const monitPrompt = async (list: boolean) => {  
    // startMaster(false)
    // .catch(() => {})
    // .finally(() => {   
    //     delaySeconds(1000)
    //     .then(() => startLedger(false))
    //     .catch(() => {})
    // })

    if(!list) return monitor();
    
    const { accounts, strategys } = await getAccountsStrategys();
    const accountStrategyList = accountStrategyListStringify(accounts, strategys);
    let answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'processName',
            message: 'Select target account / strategy process to monit    ',
            source: async (answersSoFar: any, input: string) => {
                input = input || '';
                return accountStrategyList
                    .filter((s: string) => s.indexOf(input.trim()) !== -1)
            }
        }
    ]);

    //it is so damn important, because inquirer event will conflict with blessed
    process.stdin.removeAllListeners('data'); 

    const { processName } = answers;
    const processSplit = processName.split(' ')
    const processId = processSplit[processSplit.length - 1].trim();
    const type = processSplit[0].indexOf('strategy') !== -1 ? 'strategy' : 'account';
    return tradingData(processId, type)
}



