import { getAccountSource } from '__gConfig/accountConfig';
import { requiredValidator, specialStrValidator, blankValidator, noZeroAtFirstValidator, noKeywordValidatorBuilder, chineseValidator } from '__assets/validator';
import { getTdList, addMd, addTd, updateTdConfig, getExistedMdSources, updateMdConfig } from '__io/db/account';
import { getStrategyList, addStrategy, updateStrategyPath } from '__io/db/strategy';
import { parseSources } from '@/assets/scripts/utils';
import { getKungfuTypeFromString } from '@/assets/scripts/actions';

const os = require('os');
const colors = require('colors');
const inquirer = require( 'inquirer' );
const { PathPrompt } = require('@/assets/static/js/inquirer-path/lib');
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));
inquirer.registerPrompt('path', PathPrompt);

export const selectAccountOrStrategy = async() => {
    const answers = await inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'type',
            message: 'Select targeted md, td or strategy to add    ',
            source: async () => {
                return [
                    colors.yellow('md'), 
                    colors.cyan('td'), 
                    colors.blue('strategy')
                ]
            }
        }
    ])

    return getKungfuTypeFromString(answers.type)
}

// ======================= add account start ============================

const getAvailableSources = (accountSource: Sources, existedSource: string[]) => {
    // 需要过滤含有 existedSource 的柜台
    existedSource.forEach((es: string) => {
        if(accountSource[es] !== undefined) {
            delete accountSource[es]
        }
    })
    const availSources = parseSources(accountSource)
    return availSources;
}

const selectSourcePrompt = async (accountSource: Sources, existedSource: string[]) => {
    const availSources =  getAvailableSources(accountSource, existedSource);
    if(!availSources.length) {
        console.log('No available sources, all the sources has been used!')
        process.exit(0)
    }

    return inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'source',
            message: 'Select one type of source    ',
            source: async (answersSoFar: any, input = '') => {
                return availSources
                    .filter((s: string) => s.indexOf(input) !== -1);
            }
        }
    ])
} 

export const accountConfigPrompt = (accountSetting: AccountSetting, updateModule?: boolean, accountData?: any, type?: string): Promise<any> => {
    const idKey = accountSetting.key;
    const accountOptions: AccountSettingItem[] = accountSetting.config;
    const source = accountSetting.name;
    const questions = accountOptions
    .filter((a: AccountSettingItem) => (type === 'md') || !(updateModule && a.key === idKey))
    .map((a: AccountSettingItem) => paresAccountQuestion({
        idKey,
        configItem: a,
        updateModule,
        accountData
    }, type))

    return inquirer.prompt(questions)
        .then((answers: any) => ({
            source,
            key: idKey,
            config: answers
        }))
}

export const addUpdateTdByPrompt = async (source: string, key: string, config: any, updateModule = false) => {
    if(!key) throw new Error('Something wrong with the key!')
    const accountId = `${source}_${config[key]}`
    try {
        if(updateModule) {
            await updateTdConfig(accountId, JSON.stringify(config))
            console.success(`Update ${colors.cyan('td')} ${colors.bold(accountId)} ${JSON.stringify(config, null , '')}`)   
        } else {
            await addTd(accountId, source, JSON.stringify(config))
            console.success(`Add ${colors.cyan('td')} ${colors.bold(accountId)} ${JSON.stringify(config, null , '')}`)   
        }
    } catch(err) {
        throw err
    }
}

export const addUpdateMdByPrompt = async (source: string, config: any, updateModule = false) => {
    try {
        if(updateModule) {
            await updateMdConfig(source, JSON.stringify(config))
            console.success(`Update ${colors.yellow('md')} ${colors.bold(source)} ${JSON.stringify(config, null , '')}`)   
        } else {
            await addMd(source, JSON.stringify(config))
            console.success(`Add ${colors.yellow('md')} ${colors.bold(source)} ${JSON.stringify(config, null , '')}`)   
        }
    } catch (err) {
        throw err
    }
}

const addTdPrompt = (accountSource: Sources) => {
    return selectSourcePrompt(accountSource, [])
    .then(({ source }: { source: string }) => source.split(' ')[0])
    .then((source: string) => {
        const accountSetting: AccountSetting = accountSource[source];
        if(!accountSetting) throw new Error(`No ${source} config information!`)
        return accountConfigPrompt(accountSetting)
    })
    .then(({ source, key, config }: { source: string, key: string, config: any}) => {
        config = filterAccountConfig(config);
        return addUpdateTdByPrompt(source, key, config)
    })
}

const addMdPrompt = (accountSource: Sources) => {
    return getExistedMdSources()
    .then((sourcesList: string[]) => selectSourcePrompt(accountSource, sourcesList))
    .then(({ source }: { source: string }) => source.split(' ')[0]) 
    .then((source: string) => {
        const accountSetting: AccountSetting = accountSource[source];
        if(!accountSetting) throw new Error(`No ${source} config information!`)
        return accountConfigPrompt(accountSetting, false, null, 'md')
    })
    .then(({ source, key, config }: { source: string, key: string, config: any}) => {
        config = filterAccountConfig(config);
        return addUpdateMdByPrompt(source, config)
    })
}

// ======================= add account end ============================

// ======================= add strategy start ============================
export const addUpdateStrategyPrompt = async (strategyData?: any, updateModule?: boolean): Promise<any> => {
    const { strategy_id, strategy_path } = await inquirer.prompt(buildStrategyQuestion(strategyData, updateModule).filter(q => !!q))

    try {
        if(updateModule) {
            const strategyId = strategyData.strategy_id;
            await updateStrategyPath(strategyId, strategy_path)
            console.success(`Update ${colors.blue('strategy')} ${colors.bold(strategyId)} ${strategy_path}`)
        } else {
            await addStrategy(strategy_id, strategy_path)
            console.success(`Add ${colors.blue('strategy')} ${colors.bold(strategy_id)} ${strategy_path}`)
        }
    }catch(err) {
        throw err;
    }
}

// ======================= add strategy end ============================



function buildStrategyQuestion(strategyData: any, updateModule: boolean | undefined) {
    if(updateModule) {
        return [{
            type: 'path',
            name: 'strategy_path',
            default: strategyData.strategy_path || os.homedir(),
            message: `Update strategy_path (absolute path)`,
            validate: (value: string) => {
                let hasError = null;
                !updateModule && requiredValidator(null, value, (err: Error) => hasError = err)
                if(hasError) return hasError
                else return true;
            }
        }]
    }

    return [
        {
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
                    noKeywordValidatorBuilder('all'),
                    noKeywordValidatorBuilder('ledger'),
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
        }, {
            type: 'path',
            name: 'strategy_path',
            message: `Enter strategy_path (absolute path)`,
            validate: (value: string) => {
                let hasError = null;
                !updateModule && requiredValidator(null, value, (err: Error) => hasError = err)
                if(hasError) return hasError
                else return true;
            }
        }
    ]
}

export const addAccountStrategy = async (type: string): Promise<any> => {
    try {
        switch (type) {
            case 'md':
                const { md } = await getAccountSource()
                await addMdPrompt(md) 
                break;
            case 'td':
                const { td } = await getAccountSource()
                await addTdPrompt(td) 
                break;
            case 'strategy':
                await addUpdateStrategyPrompt()
                break;
        }
    } catch (err) {
        throw err
    }
    
}

function getType(originType: string) {
    switch(originType) {
        case 'str':
            return 'input';
        case 'int':
            return 'number';
        case 'float':
            return 'number';
        case 'select':
            return 'list';
        case 'bool':
            return 'confirm';
        case 'file':
            return 'path';
        default:
            return 'input'
    }
}

function getDefaultValue(updateModule: boolean | undefined, existedValue: any, targetType: string, defaultValue?: any) {
    if(updateModule) {
        if((targetType === 'path') && !existedValue) {
            return os.homedir();
        }
        return existedValue
    } else {
        if(defaultValue !== undefined) return defaultValue;
        if(targetType === 'path') return os.homedir();
        else if(targetType === 'number') return 0
    }
}

function renderSelect(configItem: AccountSettingItem) {
    if(configItem.type === 'select') return `(${(configItem.data || []).map(item => item.value || "").join('|')})`
    else return ''
}

function paresAccountQuestion({ idKey, configItem, updateModule, accountData }: { idKey: string, configItem: AccountSettingItem, updateModule?: boolean, accountData?: any }, type?: string) {
    const { validator, required, key } = configItem;
    const targetType = getType(configItem.type);
    const existedValue = (accountData || {})[key] || '';
    let questions = {
        type: targetType,
        name: key,
        choices: targetType === 'list' ? (configItem.data || []).map(item => item.value) : [],
        message: `${updateModule ? 'Update' : 'Enter'} ${key} ${renderSelect(configItem)}`,
        validate: async (value: string | number) => {
            let hasError: Error | null = null; 
            const validatorList: Array<any>  = [...(validator || []), (required && !updateModule) ? requiredValidator : null]
            validatorList
                .filter((v: Function | null): boolean => !!v)
                .forEach((v: Function) => {
                    v(null, value, (err: Error) => {
                        if(err) hasError = err;
                    })
                })

            //判断是否关键字重复
            if((idKey === key) && (type !== 'md')) {
                const existedAccountError = await existedAccountIdValidator(value)
                if(existedAccountError) hasError = existedAccountError;
            }

            if(hasError) return hasError
            else return true;
        }
    }
    
    const defaultValue = getDefaultValue(updateModule, existedValue, targetType, configItem.default)

    //如果default value 为 NaN 0 undefined 则没有d efaultvalue
    if(!!defaultValue && (defaultValue !== false) && (defaultValue !== 0)) {
        //@ts-ignore
        questions.default = defaultValue;
    }

    return questions;
}

async function existedAccountIdValidator(value: any):Promise<any> {
    const accountList = await getTdList()
    const existedIds = accountList.map((a: Account) => a.account_id.toAccountId());
    if (existedIds.indexOf(value) !== -1) return new Error('AccountId has existed!');
}

export function filterAccountConfig(config: NormalObject) {
    Object.keys(config || {}).forEach(key => {
        const str = config[key].toString().trim();
        if((str === '') || (str === 'NaN')) {
            config[key] = null
            delete config[key]
        }
    })
    return config
}
