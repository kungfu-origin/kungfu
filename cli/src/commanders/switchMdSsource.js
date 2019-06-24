import { parseSources } from '__@/assets/utils';
import { getAccountList, changeAccountMd } from '@/io/account';

const colors = require('colors');
const inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

export const switchMdSource = async () => {
    const accountList = await getAccountList()
    let mdAccountData = []; // [ xtp(accountId) ]
    let sourceAccountData = {}; // {source: [{ accountId, ... }, { accountId, ... }]}
    let oldSourceAccount = {} // { source (accountId): accountId }

    accountList.forEach(a => {
        const accountId = a.account_id;
        const sourceName = a.source_name;
        const receiveMd = a.receive_md;
        if(receiveMd) {
            const mdAccountId = `${sourceName} (${accountId.toAccountId()})`;
            mdAccountData.push(mdAccountId);
            oldSourceAccount[mdAccountId] = { accountId, sourceName };
        }
        sourceAccountData[sourceName] = sourceAccountData[sourceName] || [];
        sourceAccountData[sourceName].push(receiveMd ? `${accountId.toAccountId()} ${colors.green('md ✓')}` : accountId.toAccountId());
    })

    if(!mdAccountData.length) throw new Error(' No account in KungFu system!')
    let { accountId } = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'accountId',
            message: 'Select one source for switch md source    ',
            source: (answersSoFar, input) => {
                input = input || ''
                const selected = mdAccountData.filter(s => s.indexOf(input) !== -1)
                return new Promise(resolve => resolve(selected))
            }
        }
    ])

    const selectedAccount = oldSourceAccount[accountId]
    const selectedSource = selectedAccount.sourceName;
    const oldAccountId = selectedAccount.accountId;
    const accountListBySource = sourceAccountData[selectedSource];
    let { newAccountId } = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'newAccountId',
            message: `Select one account for source ${selectedSource} as market data recevier   `,
            source: (answersSoFar, input) => {
                input = input || ''
                const selected = accountListBySource.filter(a => a.indexOf(input) !== -1)
                return new Promise(resolve => resolve(selected))
            }
        }
    ])

    newAccountId = `${selectedSource}_${newAccountId}`;
    await changeAccountMd(oldAccountId, false);
    await changeAccountMd(newAccountId, true);
    console.log(`Change market data receiver account for ${selectedSource} from ${oldAccountId.toAccountId()} to ${newAccountId.toAccountId()} success!`)
}