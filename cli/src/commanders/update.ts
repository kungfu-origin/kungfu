import { accountConfigPrompt, addUpdateAccountByPrompt, addUpdateStrategyPrompt } from '@/commanders/add';
import { parseToString } from '@/assets/scripts/utils';
import { getAccountsStrategys } from '@/assets/scripts/actions';
import { getAccountSource } from '__gConfig/accountConfig';


const inquirer = require('inquirer');
const colors = require('colors');

export const updateAccountStrategy = async () => {
    const { accounts, strategys } = await getAccountsStrategys();
    const accountStrategyList = [
        ...accounts.map((a: Account): string => parseToString(
            [colors.cyan('account'), a.account_id],
            [8, 'auto'],
            1
        )),
        ...strategys.map((s: Strategy): string => parseToString(
            [colors.yellow('strategy'), s.strategy_id],
            [8, 'auto'],
            1
        ))
    ]

    const answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'process',
            message: 'Select targeted account / strategy to update    ',
            source: (answersSoFar: any, input: string) => {
                input = input || ''
                const selected = accountStrategyList.filter((s: string): boolean => s.indexOf(input) !== -1)
                return new Promise(resolve => resolve(selected))
            }
        }
    ])

    const processes = answers.process;
    const splits = processes.split(" ");
    const targetType = splits[0].trim();
    const targetId = splits[splits.length - 1].trim();
    const type = targetType.indexOf('strategy') !== -1 
    ? 'strategy' 
    : targetType.indexOf('account') !== -1 
        ? 'account' 
        : undefined;
    
    if(type === 'account') {
        const targetAccount = accounts.filter((a: Account) => a.account_id === targetId)
        const source = targetId.split('_')[0];
        const accountData = JSON.parse(targetAccount[0].config || "{}");
        const accountSource = await getAccountSource();
        const accountSetting = accountSource[source];
        const { key, config } = await accountConfigPrompt(accountSetting, true, accountData);
        Object.keys(config || {}).forEach(k => {
            if(config[k].trim() === '') {
                config[k] = null
                delete config[k]
            }
        })
        try {
            await addUpdateAccountByPrompt(source, key, {
                ...accountData,
                ...config
            }, true)
        } catch (err) {
            console.error(err)
        }
    } else if(type === 'strategy') {
        const targetStrategy = strategys.filter((s: Strategy) => s.strategy_id === targetId)
        const strategyData = targetStrategy[0] || {}
        try {
            await addUpdateStrategyPrompt(strategyData, true)
        } catch (err) {
            console.error(err)
        }
    }
}