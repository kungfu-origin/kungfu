import { getAccountsStrategys, accountStrategyListStringify } from '@/assets/scripts/actions/accountStrategyActions';
const inquirer = require('inquirer');
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

export const monitPrompt = async (list: boolean) => {  

    if(!list) {
        process.env.CLI_WINDOW_TYPE = 'monit'
        const monitor = require('@/components/monitor/index').default;
        return monitor()
    };
    
    const { mds, tds, strategies } = await getAccountsStrategys(); 
    const accountStrategyList = accountStrategyListStringify(mds, tds, strategies);


        
    let answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'processName',
            message: 'Select target account / strategy process to monit    ',
            source: async (answersSoFar: any, input: string) => {
                input = input || '';
                return accountStrategyList
                    .filter((s: string) => !s.includes('md'))
                    .filter((s: string) => s.includes(input.trim()))
            }
        }
    ]);


    //it is so damn important, because inquirer event will conflict with blessed
    process.stdin.removeAllListeners('data'); 

    const { processName } = answers;
    const processSplit = processName.split(' ')
    const processId = processSplit[processSplit.length - 1].trim();
    const type = processSplit[0].includes('strategy') ? 'strategy' : 'account';
    process.env.CLI_WINDOW_TYPE = ['monit', type, processId].join('_');

    const tradingData = require('@/components/tradingData/index').default
    return tradingData(processId, type)
}



