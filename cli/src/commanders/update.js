import { listAccountsStrategys } from './list';
import { accountConfigPrompt, addAccountByPrompt, addStrategyPrompt } from './add';
import { parseToString } from '__@/assets/utils';

const inquirer = require('inquirer');
const colors = require('colors');

export const updateAccountStrategy = async () => {
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
            message: 'Select targeted account / strategy to update    ',
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
    const type = targetType.indexOf('strategy') !== -1 ? 'strategy' 
    : targetType.indexOf('account') !== -1 ? 'account' : undefined
    
    if(type === 'account') {
        const targetAccount = accounts.filter(a => a.account_id === targetId)
        const source = targetId.split('_')[0];
        const accountData = JSON.parse(targetAccount[0].config, null, '');
        const { key, config } = await accountConfigPrompt(source, true, accountData)
        Object.keys(config || {}).forEach(k => {
            if(config[k].trim() === '') {
                config[k] = null
                delete config[k]
            }
        })
        try {
            await addAccountByPrompt(source, key, {
                ...accountData,
                ...config
            }, true)
        } catch (err) {
            console.error(err)
        }
    } else if(type === 'strategy') {
        const targetStrategy = strategys.filter(s => s.strategy_id === targetId)
        const strategyData = targetStrategy[0] || {}
        try {
            await addStrategyPrompt(strategyData, true)
        } catch (err) {
            console.error(err)
        }
    }
}