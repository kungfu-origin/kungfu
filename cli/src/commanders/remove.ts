import { getAccountsStrategys, accountStrategyListStringify, getKungfuTypeFromString } from '@/assets/scripts/actions';
import { deleteMd, deleteTd } from '__io/actions/account';
import { deleteStrat } from '__io/actions/strategy';

const colors = require('colors');
const inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

export const removeAccountStrategy = async () => {
    const { mds, tds, strategies } = await getAccountsStrategys()
    const accountStrategyList = accountStrategyListStringify(mds, tds, strategies)
    let answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'process',
            message: 'Select targeted account / strategy to remove    ',
            source: async (answersSoFar: any, input: string) => {
                input = input || '';
                return accountStrategyList
                    .filter((s: string) => s.indexOf(input) !== -1)
            }
        }
    ])
    answers = answers.process;
    const splits = answers.split(" ");
    const targetId = splits[splits.length - 1].trim();
    const targetType = splits[0].trim();
    const type = getKungfuTypeFromString(targetType)

    try {
        switch (type) {
            case 'md':
                const targetMd = mds.filter((m: Md) => m.source_name === targetId)[0] || {}
                await deleteMd(targetMd)
                console.success(`Delete ${colors.yellow(type)} ${colors.bold(targetId)}`)
                break;
            case 'td':
                const targetTd = tds.filter((a: Account) => a.account_id === targetId)[0] || {}
                await deleteTd(targetTd)
                console.success(`Delete ${colors.cyan(type)} ${colors.bold(targetId)}`)
                break;
            case 'strategy':
                await deleteStrat(targetId)
                console.success(`Delete ${colors.blue(type)} ${colors.bold(targetId)}`)
                break;
        }
    } catch (err) {
        throw err
    }

} 

