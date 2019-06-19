import { listAccountsStrategys } from './list';
import { parseToString } from '__@/assets/utils';
import { deleteAccount } from '@/io/actions/account';
import { deleteStrat } from '@/io/actions/strategy';

const colors = require('colors');
const inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

export const removeAccountStrategy = async () => {
    const { accounts, strategys } = await listAccountsStrategys(true)
    const accountStrategyList = [
        ...accounts.map(a => parseToString(
            [colors.cyan('account'), a.account_id],
            [8, 'auto'],
            1
        )),
        ...strategys.map(s => parseToString(
            [colors.yellow('strategy'), s.strategy_id],
            [8, 'auto'],
            1
        ))
    ]
    let answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'process',
            message: 'Select targeted account / strategy to remove    ',
            source: (answersSoFar, input) => {
                input = input || ''
                const selected = accountStrategyList.filter(s => s.indexOf(input) !== -1)
                return new Promise(resolve => resolve(selected))
            }
        }
    ])
    answers = answers.process;
    const splits = answers.split(" ");
    const targetType = splits[0].trim();
    const targetId = splits[splits.length - 1].trim();
    const targetAccount = accounts.filter(a => a.account_id === targetId)
    const type = targetType.indexOf('strategy') !== -1 ? 'strategy' 
        : targetType.indexOf('account') !== -1 ? 'account' : undefined
    if(type === 'strategy') {
        try{
            await deleteStrat(targetId)
            console.log(`Delete ${targetType} ${targetId} sucess !`)
        }catch(err){
            console.error(err)
        }
    }
    else if(type === 'account') {
        try{
            await deleteAccount(targetAccount[0], accounts)
            console.log(`Delete ${targetType} ${targetId} sucess !`)
        }catch(err){
            console.error(err)
        }
    } 

} 

