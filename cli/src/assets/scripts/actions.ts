import { getAccountList } from '__io/db/account';
import { getStrategyList } from '__io/db/strategy';
import { parseToString } from '@/assets/scripts/utils';
const colors = require('colors');

export const getAccountsStrategys = async (): Promise<any> => {
    const getAccounts = getAccountList();
    const getStrategys = getStrategyList();
    const accounts = await getAccounts;
    const strategys = await getStrategys;
    return {
        accounts,
        strategys
    }
}

export const accountStrategyListStringify = (accounts: Account[], strategys: Strategy[]) => {
    return [
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
}

