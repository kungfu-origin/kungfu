import { getAccountsStrategys, accountStrategyListStringify } from '@/assets/scripts/actions';
import { parseToString } from '@/assets/scripts/utils';
import { deleteAccount } from '__io/actions/account';
import { deleteStrat } from '__io/actions/strategy';

const colors = require('colors');
const inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

export const removeAccountStrategy = async () => {
    const { accounts, strategys } = await getAccountsStrategys()
    const accountStrategyList = accountStrategyListStringify(accounts, strategys)
    let answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'process',
            message: 'Select targeted account / strategy to remove    ',
            source: async (answersSoFar: any, input = '') => {
                return accountStrategyList
                    .filter((s: string) => s.indexOf(input) !== -1)
            }
        }
    ])
    answers = answers.process;
    const splits = answers.split(" ");
    const targetType = splits[0].trim();
    const targetId = splits[splits.length - 1].trim();
    const targetAccount = accounts.filter((a: Account) => a.account_id === targetId)
    const type = targetType.indexOf('strategy') !== -1 
        ? 'strategy' 
        : targetType.indexOf('account') !== -1 
            ? 'account' 
            : undefined
            
    if(type === 'strategy') {
        try{
            await deleteStrat(targetId)
            console.success(`Delete ${targetType} ${colors.bold(targetId)}`)
        }catch(err){
            console.error(err)
        }
    }
    else if(type === 'account') {
        try{
            await deleteAccount(targetAccount[0], accounts)
            console.success(`Delete ${targetType} ${colors.bold(targetId)}`)
        }catch(err){
            console.error(err)
        }
    } 

} 

