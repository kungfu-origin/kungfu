import { sourceType, accountSource } from '@/assets/config/accountConfig';
import { validateRequired, specialStrValidator, blankValidator, noZeroAtFirstValidator, chineseValidator } from '@/assets/js/validator';
import { getAccountList, getAccountBySource, addAccount, updateAccountConfig } from '@/io/db/account';
import { getStrategyList, addStrategy, updateStrategyPath } from '@/io/db/strategy';
import { parseSources } from '__@/assets/utils';

var inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));

// ======================= add account start ============================
const selectSourcePrompt = () => inquirer.prompt([
    {
        type: 'autocomplete',
        name: 'source',
        message: 'Select one type of source    ',
        source: (answersSoFar, input) => {
            input = input || ''
            const selected = parseSources(sourceType).filter(s => s.indexOf(input) !== -1)
            return new Promise(resolve => resolve(selected))
        }
    }
])

export const addAccountByPrompt = async (source, key, config, updateModule = false) => {
    if(!key) throw new Error('something wrong with the key!')
    const accountId = `${source}_${config[key]}`
    const accountsBySource = await getAccountBySource(source)
    try {
        if(updateModule) {
            await updateAccountConfig(accountId, JSON.stringify(config))
            console.log(`Update account ${JSON.stringify(config, null , '')} sucess!`)   
        }else{
            await addAccount(accountId, source, !accountsBySource.length, JSON.stringify(config))
            console.log(`Add account ${JSON.stringify(config, null , '')} sucess!`)   
        }
    }catch(err){
        throw err
    }
    return
}

export const accountConfigPrompt = (source, updateModule = false, accountData = {}) => {
    source = source.split(' ')[0] // e.g. source = 'xtp (stock)'
    const accountOptions = accountSource[source];
    const idKey = sourceType[source].key
    if(!accountOptions) throw new Error `No ${source} config information!`
    const questions = accountOptions.filter(a => !(updateModule && a.key === idKey)).map(a => paresAccountQuestion(a, source, updateModule, accountData))
    return inquirer.prompt(questions).then(answers => ({
        source,
        key: sourceType[source].key || '',
        config: answers
    }))
}

const addAccountPrompt = () => {
    return selectSourcePrompt()
    .then(({ source }) => accountConfigPrompt(source))
    .then(({ source, key, config }) => addAccountByPrompt(source, key, config))
}


// ======================= add account end ============================

// ======================= add strategy start ============================
export const addStrategyPrompt = async (strategyData, updateModule) => {
    const {strategy_id, strategy_path} = await inquirer.prompt([
        !updateModule ? {
            type: 'input',
            name: 'strategy_id',
            message: 'Enter strategy_id',
            validate: async value => {
                let hasError = null;
                [validateRequired, specialStrValidator, blankValidator, noZeroAtFirstValidator, chineseValidator].forEach(validator => {
                    if(hasError) return;
                    validator(null, value, (err) => err && (hasError = err))                    
                })
                
                const strategyList = await getStrategyList()
                const existedIds = strategyList.map(s => s.strategy_id);
                (existedIds.indexOf(value) !== -1) && (hasError = new Error('该 strategy_id 已存在！'));
                
                if(hasError) return hasError
                else return true;
            }
        } : null , {
            type: 'input',
            name: 'strategy_path',
            message: `${updateModule ? 'Update' : 'Enter'} strategy_path ${ updateModule ? `(${strategyData.strategy_path})` : '' }`,
            validate: value => {
                let hasError = null;
                !updateModule && validateRequired(null, value, (err) => hasError = err)
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


export const addAccountStrategy = () => inquirer.prompt([
    {
        type: 'list',
        name: 'add',
        message: 'Add a account or strategy    ',
        choices: ['account', 'strategy']
    }
]).then(answers => {
    const type = answers.add;
    if(type === 'account') return addAccountPrompt()
    else if(type === 'strategy') return addStrategyPrompt()
})





function paresAccountQuestion(accountConfig, source, updateModule = false, accountData = {}){
    const key = accountConfig.key;
    const idKey = sourceType[source].key
    const validators = accountConfig.validator;
    const required = accountConfig.required;
    const existedValue = accountData[key];
    return {
        type: 'input',
        name: key,
        message: `${updateModule ? 'Update' : 'Enter'} ${key} ${updateModule ? '(' + (existedValue || 'null') + ')' : ''}`,
        validate: async value => {
            let hasError = null;
            let validatorList = [];
            if(required && !updateModule) validatorList.push(validateRequired);
            [...(validators || []), ...validatorList].forEach(validator => {
                if(hasError) return;
                validator(null, value, (err) => err && (hasError = err))
            })
            
            if(idKey === key) { //id 重复
                const accountList = await getAccountList()
                const existedIds = accountList.map(a => a.account_id.toAccountId());
                (existedIds.indexOf(value) !== -1) && (hasError = new Error('该 accountId 已存在！'));
            }

            if(hasError) return hasError
            else return true;
        }
    }
}
