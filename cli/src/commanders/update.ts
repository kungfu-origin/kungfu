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

    try {
        switch (type) {
            case 'td':
                await updateTd(targetId, tds)
                break;
            case 'md':
                await updateMd(targetId, mds)
                break;
            case 'strategy':
                await updateStrategy(targetId, strategies)
                break;    
        }
    } catch (err) {
        throw err
    }
}

async function updateMd(targetSource: string, mds: Md[]) {
    const targetMd = mds.filter((m: Md) => m.source_name === targetSource)[0] || {};
    const mdConfig = JSON.parse(targetMd.config || "{}")
    const { md } = await getAccountSource();
    const accountSetting = md[targetSource];
    const { source, config } = await accountConfigPrompt(accountSetting, true, mdConfig, 'md');
    
    const configResolve = filterAccountConfig(config)
    console.log('Diff: ', configResolve)

    return await addUpdateMdByPrompt(
        source, 
        {
        ...mdConfig,
        ...configResolve
        }, 
        true
    )
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

    return await addUpdateTdByPrompt(
        source, 
        key, 
        {
            ...tdConfig,
            ...config
        }, 
        true
    )
}

async function updateStrategy(targetId: string, strategies: Strategy[]) {
    const targetStrategy = strategies.filter((s: Strategy) => s.strategy_id === targetId)
    const strategyData = targetStrategy[0] || {}
    return await addUpdateStrategyPrompt(strategyData, true)
}