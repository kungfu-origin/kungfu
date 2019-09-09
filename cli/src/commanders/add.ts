import { getAccountSource } from '__gConfig/accountConfig';
import * as VALIDATOR from '__assets/validator';
import { requiredValidator, specialStrValidator, blankValidator, noZeroAtFirstValidator, chineseValidator } from '__assets/validator';
import { getAccountList, getAccountBySource, addAccount, updateAccountConfig } from '__io/db/account';
import { getStrategyList, addStrategy, updateStrategyPath } from '__io/db/strategy';
import { parseSources } from '@/assets/utils';

var inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));



// ======================= add account start ============================
const selectSourcePrompt = (accountSource: Sources) => inquirer.prompt([
    {
        type: 'autocomplete',
        name: 'source',
        message: 'Select one type of source    ',
        source: async (answersSoFar: any, input = '') => {
            return parseSources(accountSource)
            .filter((s: string) => s.indexOf(input) !== -1);
        }
    }
])

// export const addAccountByPrompt = async (source: string, key: string, config: any, updateModule = false) => {
//     if(!key) throw new Error('something wrong with the key!')
//     const accountId = `${source}_${config[key]}`
//     const accountsBySource = await getAccountBySource(source)
//     try {
//         if(updateModule) {
//             await updateAccountConfig(accountId, JSON.stringify(config))
//             console.log(`Update account ${JSON.stringify(config, null , '')} sucess!`)   
//         }else{
//             await addAccount(accountId, source, !accountsBySource.length, JSON.stringify(config))
//             console.log(`Add account ${JSON.stringify(config, null , '')} sucess!`)   
//         }
//     }catch(err){
//         throw err
//     }
//     return
// }

export const accountConfigPrompt = (accountSetting: AccountSetting, updateModule?: boolean, accountData?: any ): Promise<any> => {
    const idKey = accountSetting.key;
    const accountOptions = accountSetting.config;
    const source = accountSetting.name;
    const questions = accountOptions
    .filter((a: AccountSettingItem) => !(updateModule && a.key === idKey))
    .map((a: AccountSettingItem) => paresAccountQuestion({
        idKey,
        configItem: a,
        updateModule,
        accountData
    }))

    return inquirer.prompt(questions).then((answers: any) => ({
        source,
        key: idKey,
        config: answers
    }))
}

const addAccountPrompt = (accountSource: Sources) => {
    return selectSourcePrompt(accountSource)
    .then(({ source }: { source: string }) => source.split(' ')[0])
    .then((source: string) => {
        const accountSetting: AccountSetting = accountSource[source];
        if(!accountSetting) throw new Error(`No ${source} config information!`)
        return accountConfigPrompt(accountSetting)
    })
    

    // .then(({ source, key, config }: { source: string, key: string, config: any}) => addAccountByPrompt(source, key, config))
}


// ======================= add account end ============================

// ======================= add strategy start ============================
export const addStrategyPrompt = async (strategyData: any, updateModule: boolean): Promise<any> => {
    const {strategy_id, strategy_path} = await inquirer.prompt([
        !updateModule ? {
            type: 'input',
            name: 'strategy_id',
            message: 'Enter strategy_id',
            validate: async (value: any) => {
                let hasError: Error | null  = null;
                [requiredValidator, specialStrValidator, blankValidator, noZeroAtFirstValidator, chineseValidator].forEach(validator => {
                    if(hasError) return;
                    validator(null, value, (err: Error) => err && (hasError = err))                    
                })
                
                const strategyList = await getStrategyList()
                const existedIds = strategyList.map((s: Strategy) => s.strategy_id);
                (existedIds.indexOf(value) !== -1) && (hasError = new Error('该 strategy_id 已存在！'));
                
                if(hasError) return hasError
                else return true;
            }
        } : null , {
            type: 'input',
            name: 'strategy_path',
            message: `${updateModule ? 'Update' : 'Enter'} strategy_path ${ updateModule ? `(${strategyData.strategy_path})` : '' }`,
            validate: (value: any) => {
                let hasError = null;
                !updateModule && requiredValidator(null, value, (err: Error) => hasError = err)
                if(hasError) return hasError
                else return true;
            }
        }
    ].filter(q => !!q))
    try {
        if(updateModule) {
            const strategyId = strategyData.strategy_id;
            await updateStrategyPath(strategyId, strategy_path)
            console.log(`Update strategy ${strategyId} ${strategy_path} sucess!`)

        } else {
            await addStrategy(strategy_id, strategy_path)
            console.log(`Add strategy ${strategy_id} ${strategy_path} sucess!`)
        }
       
    }catch(err) {
        throw err;
    }
    return
}

// ======================= add strategy end ============================


export const addAccountStrategy = async (type: string): Promise<any> => {
    if(type === 'account') {
        try {
            const accountSource = await getAccountSource()
            return await addAccountPrompt(accountSource)
        } catch (err) {
            throw err
        }
    }
    // else if(type === 'strategy') return addStrategyPrompt()
}




function paresAccountQuestion({
    idKey, configItem, updateModule, accountData
}: {
    idKey: string,
    configItem: AccountSettingItem,
    updateModule?: boolean,
    accountData?: any
}) {
    const { validator, required, key } = configItem 
    const existedValue = (accountData || {})[key] || '';
    return {
        type: 'input',
        name: key,
        message: `${updateModule ? 'Update' : 'Enter'} ${key} ${updateModule ? '(' + (existedValue || 'null') + ')' : ''}`,
        validate: async (value: any) => {
            const validatorList: Array<any>  = [
                //@ts-ignore
                ...(validator || []).map((v: string) => (VALIDATOR[v] || null)),
                (required && !updateModule) ? VALIDATOR.requiredValidator : null
            ]

            let hasError: Error | null = null; 
            validatorList
            .filter((v: Function | null): boolean => !!v)
            .forEach((v: Function) => {
                v(null, value, (err: Error) => {
                    if(err) hasError = err;
                })
            })

            if(idKey === key) {
                const existedAccountError = await existedAccountIdValidator(value)
                if(existedAccountError) hasError = existedAccountError;
            }

            if(hasError) return hasError
            else return true;
        }
    }
}


async function existedAccountIdValidator(value: any):Promise<any> {
    const accountList = await getAccountList()
    const existedIds = accountList.map((a: Account) => a.account_id.toAccountId());
    if (existedIds.indexOf(value) !== -1) return new Error('accountId is existed!');
}