import { getAccountsStrategys } from '@/assets/scripts/actions';
import { parseToString } from '@/assets/scripts/utils';
const colors = require('colors');

export const listAccountsStrategys = async (): Promise<any> => {
    const { mds, tds, strategys } = await getAccountsStrategys();

    const mdList = mds.map((m: Md): string => {
        return parseToString(
            [colors.yellow('Md'), colors.bold(m.source_name), m.config],
            [10, 15, 'auto'],
            1
        )
    })

    const tdList = tds.map((a: Account): string => {
        return parseToString(
            [colors.cyan('Td'), colors.bold(a.account_id),  a.config],
            [10, 15, 'auto'],
            1
        )
    })
    const strategyList = strategys.map((s: Strategy): string => {
        return parseToString(
            [colors.blue('strategy'), colors.bold(s.strategy_id), s.strategy_path],
            [10, 15, 'auto'],
            1
        )
    })

    if([...mdList, ...tdList, ...strategyList].length === 0) {
        console.log('No mds, tds and strategies!')
    } else {
        console.log([
            `\n${colors.bold.underline('Md:')} \n`,
            ...mdList,
            `\n${colors.bold.underline('Td:')} \n`,
            ...tdList,
            `\n${colors.bold.underline('Strategy:')} \n`, 
            ...strategyList
        ].join('\n'))
    }
}