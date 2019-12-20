import { accountConfigPrompt, addUpdateTdByPrompt, addUpdateStrategyPrompt } from '@/commanders/add';
import { getAccountsStrategys, accountStrategyListStringify, getKungfuTypeFromString } from '@/assets/scripts/actions';
import { getAccountSource } from '__gConfig/accountConfig';
const inquirer = require('inquirer');

export const updateAccountStrategy = async () => {
    const { mds, tds, strategys } = await getAccountsStrategys();
    const accountStrategyList = accountStrategyListStringify(mds, tds, strategys)
    const answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'process',
            message: 'Select targeted md / td / strategy to update    ',
            source: async (answersSoFar: any, input: string) => {
                input = input || '';
                return accountStrategyList
                    .filter((s: string): boolean => s.indexOf(input) !== -1)
            }
        }
    ])

    const processes = answers.process;
    const splits = processes.split(" ");
    const targetType = splits[0].trim();
    const targetId = splits[splits.length - 1].trim();
    const type = getKungfuTypeFromString(targetType)

    switch (type) {
        case 'td':
            const targetAccount = tds.filter((a: Account) => a.account_id === targetId)
            const source = targetId.split('_')[0];
            // const accountData = JSON.parse(targetAccount[0].config || "{}");
            // const accountSource = await getAccountSource();
            // const accountSetting = accountSource[source];
            // const { key, config } = await accountConfigPrompt(accountSetting, true, accountData);

            // Object.keys(config || {}).forEach(k => {
            //     const str = config[k].toString().trim();
            //     if (str === '' || str === 'NaN') {
            //         config[k] = null
            //         delete config[k]
            //     }
            // })

            // console.log('Diff: ', config)

            // try {
            //     await addUpdateTdByPrompt(
            //         source, 
            //         key, 
            //         {
            //             ...accountData,
            //             ...config
            //         }, 
            //         true
            //     )
            // } catch (err) {
            //     console.error(err)
            // }
            break;
        case 'md':
            break;
        case 'strategy':
            const targetStrategy = strategys.filter((s: Strategy) => s.strategy_id === targetId)
            const strategyData = targetStrategy[0] || {}
            try {
                await addUpdateStrategyPrompt(strategyData, true)
            } catch (err) {
                console.error(err)
            }
            break;
    }
}