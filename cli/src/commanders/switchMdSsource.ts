import { parseSources } from '@/assets/scripts/utils';
import { getAccountList, changeAccountMd } from '__io/db/account';

const colors = require('colors');
const inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

interface OldSourceAccount {
    [ propName: string ]: { 
        accountId: string, 
        sourceName: string
    }
}

export const switchMdSource = async () => {
    const accountList: Account[] = await getAccountList()
    const mdAccountData = accountList
        .filter((a: Account) => a.receive_md)
        .map((a: Account) => `${a.source_name} (${a.account_id.toAccountId()})`)

    if(!mdAccountData.length) throw new Error('No account in KungFu system!')

    const oldSourceAccount: OldSourceAccount = [{}, ...accountList.filter((a: Account) => a.receive_md)]
        .reduce((a: any, b: any): any => {
            const mdAccountId = `${b.source_name} (${b.account_id.toAccountId()})`;
            a[mdAccountId] = {
                accountId: b.account_id,
                sourceName: b.source_name
            }
            return a;
        })
               

    const sourceAccountData: { [propName: string]: string[] } = [{}, ...accountList]
        .reduce((a: any, b: any): any => {
            const receiveMd = b.receive_md;
            const sourceName = b.source_name;
            const accountId = b.account_id;

            a[sourceName] = a[sourceName] || [];
            a[sourceName].push(
                receiveMd 
                    ? `${accountId.toAccountId()} ${colors.green('md ✓')}`
                    : `${accountId.toAccountId()}`
            )
            return a;
        })

    const { accountId } = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'accountId',
            message: 'Select one source for switch md source    ',
            source: async (answersSoFar: any, input = '') => {
                return mdAccountData.filter(s => s.indexOf(input) !== -1)
            }
        }
    ])

    const selectedAccount = oldSourceAccount[accountId]
    const selectedSource = selectedAccount.sourceName;
    const oldAccountId = selectedAccount.accountId;
    const accountListBySource = sourceAccountData[selectedSource];

    const { newAccountId } = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'newAccountId',
            message: `Select one account for source ${selectedSource} as market data recevier   `,
            source: async (answersSoFar: any, input = '') => {
                return accountListBySource.filter(a => a.indexOf(input) !== -1)
            }
        }
    ])


    const newAccountIdResolved = `${selectedSource}_${newAccountId}`;
    if(newAccountIdResolved.indexOf(oldAccountId) === -1) {
        await changeAccountMd(oldAccountId, false);
        await changeAccountMd(newAccountIdResolved, true);
    }
    console.log(`Change market data receiver account for ${selectedSource} from ${oldAccountId.toAccountId()} to ${newAccountIdResolved.toAccountId()} success!`)
}