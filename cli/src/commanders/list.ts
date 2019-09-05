import { getAccountList } from '__io/db/account';
import { getStrategyList } from '__io/db/strategy';
import { parseToString } from '@/assets/utils';

const colors = require('colors');

export const listAccountsStrategys = async (asUtil: boolean): Promise<any> => {

    const getAccounts = getAccountList();
    const getStrategys = getStrategyList();
    const accounts = await getAccounts;
    const strategys = await getStrategys;

    if(asUtil) return { accounts, strategys };
    const accountsList = accounts.map((a: Account): string[] => {
        const asMd = !!a.receive_md ? colors.green('md ✓') : ''
        return parseToString(
            [colors.cyan('account'), colors.bold(a.account_id), a.source_name, asMd,   a.config],
            [10, 12, 5, 10, 'auto'],
            1
        )
    })
    const strategyList = strategys.map((s: Strategy): string[] => {
        return parseToString(
            [colors.yellow('strategy'), colors.bold(s.strategy_id), s.strategy_path],
            [10, 12, 'auto'],
            1
        )
    })

    if(!accountsList.length && !strategyList.length) {
        console.log('还未添加账户及策略！')
    } else {
        console.log([...accountsList, ...strategyList].join('\n'))
    }
}