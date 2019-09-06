import { getAccountList } from '__io/db/account';
import { getStrategyList } from '__io/db/strategy';
import { parseToString } from '@/assets/utils';

const colors = require('colors');

export const listAccountsStrategys = async (type = ''): Promise<any> => {

    const getAccounts = getAccountList();
    const getStrategys = getStrategyList();
    const accounts = await getAccounts;
    const strategys = await getStrategys;

    const accountsList = accounts.map((a: Account): string => {
        const asMd: string = !!a.receive_md ? colors.green('md ✓') : ''
        return parseToString(
            [colors.cyan('account'), colors.bold(a.account_id), a.source_name, asMd, a.config],
            [10, 12, 5, 10, 'auto'],
            1
        )
    })
    const strategyList = strategys.map((s: Strategy): string => {
        return parseToString(
            [colors.yellow('strategy'), colors.bold(s.strategy_id), s.strategy_path],
            [10, 12, 'auto'],
            1
        )
    })

    switch (type) {
        case 'strategys':
            if(!strategyList.length) {
                console.log('no strategys!')
            } else {
                console.log([...strategyList].join('\n'))
            }
            break;
        case 'accounts':
            if(!accountsList.length) {
                console.log('no accounts!')
            } else {
                console.log([...accountsList].join('\n'))
            }
            break;
        default: 
            if(!accountsList.length && !strategyList.length) {
                console.log('no accounts and strategys!')
            } else {
                console.log([...accountsList, ...strategyList].join('\n'))
            }
    }

    return { accounts, strategys };
}