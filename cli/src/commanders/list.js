import { getAccountList } from '@/io/account';
import { getStrategyList } from '@/io/strategy';
import { parseToString } from '__@/assets/utils';
const colors = require('colors');

export const listAccountsStrategys = async (asUtil) => {

    const getAccounts = getAccountList();
    const getStrategys = getStrategyList();
    const accounts = await getAccounts;
    const strategys = await getStrategys;

    if(asUtil) return { accounts, strategys };

    const accountsList = accounts.map(a => {
        const asMd = !!a.receive_md ? colors.green('md ✓') : ''
        return parseToString(
            [colors.cyan('account'), colors.bold(a.account_id), a.source_name, asMd,   a.config],
            [10, 12, 5, 10, 'auto'],
            1
        )
    })
    
    const strategyList = strategys.map(s => {
        return parseToString(
            [colors.yellow('strategy'), colors.bold(s.strategy_id), s.strategy_path],
            [10, 12, 'auto'],
            1
        )
    })

    console.log([...accountsList, ...strategyList].join('\n'))

}