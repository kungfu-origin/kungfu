import { sourceType, accountSource } from '@/assets/config/accountConfig';
import { validateRequired, specialStrValidator, blankValidator, noZeroAtFirstValidator, chineseValidator } from '@/assets/js/validator';
import { getAccountList, getAccountBySource, addAccount } from '@/io/account';
import { getStrategyList, addStrategy } from '@/io/strategy';

const ora = require('ora');
var inquirer = require( 'inquirer' );
inquirer.registerPrompt('autocomplete', require('inquirer-autocomplete-prompt'));


// ======================= add account start ============================
const selectSourcePrompt = () => inquirer.prompt([
    {
        type: 'autocomplete',
        name: 'source',
        message: 'Select one type of source',
        // choices: parseSources(sourceType),
        source: (answersSoFar, input) => {
            input = input || ''
            const selected = parseSources(sourceType).filter(s => s.indexOf(input) !== -1)
            return new Promise(resolve => resolve(selected))
        }
    }
])

const accountConfigPrompt = (source) => {
    source = source.source.split(' ')[0] // e.g. source = 'xtp (stock)'
    const accountOptions = accountSource[source];
    if(!accountOptions) throw new Error `No ${source} config information!`
    const questions = accountOptions.map(a => paresAccountQuestion(a, source))
    return inquirer.prompt(questions).then(answers => ({
        source,
        key: sourceType[source].key || '',
        config: answers
    }))
}

const addAccountPrompt = () => {
    return selectSourcePrompt()
    .then(source => accountConfigPrompt(source))
    .then(({source, key, config}) => addAccountByPrompt(source, key, config))
}


// ======================= add account end ============================

// ======================= add strategy start ============================
const addStrategyPrompt =() => {
    return inquirer.prompt([
        {
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
        }, {
            type: 'input',
            name: 'strategy_path',
            message: 'Enter a strategy_path',
            validate: value => {
                let hasError = null;
                validateRequired(null, value, (err) => hasError = err)
                if(hasError) return hasError
                else return true;
            }
        }
    ]).then(async ({strategy_id, strategy_path}) => {
        try {
            await addStrategy(strategy_id, strategy_path)
            console.log(`Add strategy ${strategy_id} ${strategy_path} sucess!`)
        }catch(err) {
            throw err;
        }
    })
}

// ======================= add strategy end ============================


export const addStrategyOrAccount = () => inquirer.prompt([
    {
        type: 'list',
        name: 'add',
        message: 'Add a account or strategy',
        choices: ['account', 'strategy']
    }
]).then(answers => {
    const type = answers.add;
    if(type === 'account') return addAccountPrompt()
    else if(type === 'strategy') return addStrategyPrompt()
})





function parseSources(sourcesData){
    return Object.values(sourcesData).map(s => `${s.source} (${s.typeName})`)
}

function paresAccountQuestion(accountData, source){
    const key = accountData.key;
    const idKey = sourceType[source].key
    const validators = accountData.validator;
    const required = accountData.required;
    return {
        type: 'input',
        name: key,
        message: `Enter ${key}`,
        validate: async value => {
            let hasError = null;
            let validatorList = [];
            if(required) validatorList.push(validateRequired);
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

async function addAccountByPrompt(source, key, config){
    if(!key) throw new Error('something wrong with the key!')
    const accountId = `${source}_${config[key]}`
    const accountsBySource = await getAccountBySource(source)
    try {
        await addAccount(accountId, source, !accountsBySource.length, config)
        console.log(`Add account ${JSON.stringify(config, null , '')} sucess!`)   
    }catch(err){
        throw err
    }
}