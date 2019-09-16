import { getAccountsStrategys } from '@/assets/scripts/actions';
import { parseToString } from '@/assets/scripts/utils';
const colors = require('colors');

export const listAccountsStrategys = async (): Promise<any> => {
    const { accounts, strategys } = await getAccountsStrategys()
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

    if(!accountsList.length && !strategyList.length) {
        console.log('No accounts and strategies!')
    } else {
        console.log([
            `\n${colors.bold.underline('Accounts:')} \n`,
            ...accountsList, 
            `\n${colors.bold.underline('Strategies:')} \n`, 
            ...strategyList
        ].join('\n'))
    }
}