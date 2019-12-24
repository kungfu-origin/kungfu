import { accountConfigPrompt, addUpdateTdByPrompt, addUpdateMdByPrompt, addUpdateStrategyPrompt, filterAccountConfig } from '@/commanders/add';
import { getAccountsStrategys, accountStrategyListStringify, getKungfuTypeFromString } from '@/assets/scripts/actions';
import { getAccountSource } from '__gConfig/accountConfig';

const inquirer = require('inquirer');

export const updateAccountStrategy = async () => {
    const { mds, tds, strategies } = await getAccountsStrategys();
    const accountStrategyList = accountStrategyListStringify(mds, tds, strategies)
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
            updateTd(targetId, tds)
            break;
        case 'md':
            updateMd(targetId, mds)
            break;
        case 'strategy':
            updateStrategy(targetId, strategies)
            break;    
    }
}

async function updateMd(targetSource: string, mds: Md[]) {
    const targetMd = mds.filter((m: Md) => m.source_name === targetSource)[0] || {};
    const mdConfig = JSON.parse(targetMd.config || "{}")
    const { md } = await getAccountSource();
    const accountSetting = md[targetSource];
    const { source, config } = await accountConfigPrompt(accountSetting, true, mdConfig);
    const configResolve = filterAccountConfig(config)
    return addUpdateMdByPrompt(source, configResolve)
}

async function updateTd(targetId: string, tds: Account[]) {
    const targetTd = tds.filter((a: Account) => a.account_id === targetId)[0] || {};
    const source = targetId.split('_')[0];
    const tdConfig = JSON.parse(targetTd.config || "{}");
    const { td } = await getAccountSource();
    const accountSetting = td[source];
    const { key, config } = await accountConfigPrompt(accountSetting, true, tdConfig);
    
    const configResolve = filterAccountConfig(config)

    console.log('Diff: ', configResolve)

    try {
        await addUpdateTdByPrompt(
            source, 
            key, 
            {
                ...tdConfig,
                ...config
            }, 
            true
        )
    } catch (err) {
        console.error(err)
    }
}

async function updateStrategy(targetId: string, strategies: Strategy[]) {
    const targetStrategy = strategies.filter((s: Strategy) => s.strategy_id === targetId)
    const strategyData = targetStrategy[0] || {}
    try {
        await addUpdateStrategyPrompt(strategyData, true)
    } catch (err) {
        console.error(err)
    }
}