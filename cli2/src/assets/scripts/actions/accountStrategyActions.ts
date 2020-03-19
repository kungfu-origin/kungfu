import { getTdList, getMdList } from '__io/kungfu/account';
import { getStrategyList } from '__io/kungfu/strategy';
import { parseToString } from '@/assets/scripts/utils';

const colors = require('colors');

export const getAccountsStrategys = async (): Promise<any> => {
    const getTds = getTdList();
    const getMds = getMdList();
    const getStrategys = getStrategyList();
    const tds = await getTds;
    const mds = await getMds;
    const strategies = await getStrategys;
    return {
        mds,
        tds,
        strategies
    }
}

export const accountStrategyListStringify = (mds: Md[], tds: Td[], strategies: Strategy[]) => {
    return [
        ...mds.map((m: Md): string => parseToString(
            [colors.yellow('md'), m.source_name],
            [8, 'auto'],
            1
        )),
        ...tds.map((a: Td): string => parseToString(
            [colors.cyan('td'), a.account_id],
            [8, 'auto'],
            1
        )),
        ...strategies.map((s: Strategy): string => parseToString(
            [colors.blue('strategy'), 
            s.strategy_id],
            [8, 'auto'],
            1
        ))
    ]
}