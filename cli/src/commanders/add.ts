import { getAccountSource } from '__gConfig/accountConfig';
import { requiredValidator, specialStrValidator, blankValidator, noZeroAtFirstValidator, chineseValidator } from '__assets/validator';
import { getAccountList, getAccountBySource, addAccount, updateAccountConfig } from '__io/db/account';
import { getStrategyList, addStrategy, updateStrategyPath } from '__io/db/strategy';
import { parseSources } from '@/assets/scripts/utils';

const os = require('os');
const colors = require('colors');
const inquirer = require( 'inquirer' );
const { PathPrompt } = require('inquirer-path');
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));
inquirer.registerPrompt('path', PathPrompt);

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

    return inquirer.prompt(questions)
        .then((answers: any) => ({
            source,
            key: idKey,
            config: answers
        }))
}

export const addUpdateAccountByPrompt = async (source: string, key: string, config: any, updateModule = false) => {
    if(!key) throw new Error('something wrong with the key!')
    const accountId = `${source}_${config[key]}`
    const accountsBySource = await getAccountBySource(source)

    try {
        if(updateModule) {
            await updateAccountConfig(accountId, JSON.stringify(config))
            console.log(`Update ${colors.cyan('account')} ${colors.bold(accountId)} ${JSON.stringify(config, null , '')} successfully!`)   
        }else{
            await addAccount(accountId, source, !accountsBySource.length, JSON.stringify(config))
            console.log(`Add ${colors.cyan('account')} ${colors.bold(accountId)} ${JSON.stringify(config, null , '')} successfully!`)   
        }
    }catch(err){
        throw err
    }
}

const addAccountPrompt = (accountSource: Sources) => {
    return selectSourcePrompt(accountSource)
    .then(({ source }: { source: string }) => source.split(' ')[0])
    .then((source: string) => {
        const accountSetting: AccountSetting = accountSource[source];
        if(!accountSetting) throw new Error(`No ${source} config information!`)
        return accountConfigPrompt(accountSetting)
    })
    .then(({ source, key, config }: { source: string, key: string, config: any}) => {
        return addUpdateAccountByPrompt(source, key, config)
    })
}


// ======================= add account end ============================

// ======================= add strategy start ============================
export const addUpdateStrategyPrompt = async (strategyData?: any, updateModule?: boolean): Promise<any> => {
    const { strategy_id, strategy_path } = await inquirer.prompt([
        !updateModule ? {
            type: 'input',
            name: 'strategy_id',
            message: 'Enter strategy_id',
            validate: async (value: string) => {
                let hasError: Error | null  = null;
                [
                    requiredValidator, 
                    specialStrValidator, 
                    blankValidator, 
                    noZeroAtFirstValidator, 
                    chineseValidator
                ].forEach(validator => {
                    if(hasError) return;
                    validator(null, value, (err: Error) => err && (hasError = err))                    
                })
                
                const strategyList = await getStrategyList()
                const existedIds = strategyList.map((s: Strategy) => s.strategy_id);
                (existedIds.indexOf(value) !== -1) && (hasError = new Error('Strategy_id has existed!'));
                
                if(hasError) return hasError
                else return true;
            }
        } : null , {
            type: 'path',
            name: 'strategy_path',
            default: os.homedir(),
            message: `${updateModule ? 'Update' : 'Enter'} strategy_path (absolute path) ${ updateModule ? `(${strategyData.strategy_path})` : '' }`,
            validate: (value: string) => {
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
            console.log(`Update ${colors.blue('strategy')} ${colors.bold(strategyId)} ${strategy_path} sucess!`)
        } else {
            await addStrategy(strategy_id, strategy_path)
            console.log(`Add ${colors.blue('strategy')} ${colors.bold(strategy_id)} ${strategy_path} sucess!`)
        }
       
    }catch(err) {
        throw err;
    }
}

// ======================= add strategy end ============================


export const addAccountStrategy = async (type: string): Promise<any> => {
    if(type === 'account') {
        try {
            const accountSource = await getAccountSource()
            await addAccountPrompt(accountSource)
        } catch (err) {
            throw err
        }
    }
    else if(type === 'strategy') {
        try {
            await addUpdateStrategyPrompt()
        } catch (err) {
            throw err
        }
    } 
}

function getType(originType: string) {
    switch(originType) {
        case 'str':
            return 'input';
        case 'int':
            return 'number';
        case 'select':
            return 'list';
        case 'bool':
            return 'confirm';
        default:
            return 'input'
    }
}

function renderSelect(configItem: AccountSettingItem) {
    if(configItem.type === 'select') return `(${(configItem.data || []).map(item => item.value || "").join('|')})`
    else return ''
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
    const targetType = getType(configItem.type);

    return {
        type: targetType,
        name: key,
        choices: targetType === 'list' ? (configItem.data || []).map(item => item.value) : [],
        message: `${updateModule ? 'Update' : 'Enter'} ${key} ${renderSelect(configItem)} ${updateModule ? '(' + (existedValue || 'null') + ')' : ''}`,
        validate: async (value: string | number) => {
            let hasError: Error | null = null; 
            const validatorList: Array<any>  = [
                ...(validator || []),
                (required && !updateModule) ? requiredValidator : null
            ]

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
    if (existedIds.indexOf(value) !== -1) return new Error('AccountId has existed!');
}
