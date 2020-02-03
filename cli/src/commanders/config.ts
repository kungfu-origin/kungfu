import { SystemConfigItem, SystemConfigChildNormalItem, mergeSystemConfigTogether, getValueByConfigItem, setValueByConfigItem } from '__gConfig/systemConfig';
import { requiredValidator } from '__assets/validator';
import { getQuestionInputType } from '@/assets/scripts/utils';
import { getSourceList } from "__gUtils/busiUtils";

const inquirer = require( 'inquirer' );

const systemConfig = mergeSystemConfigTogether();


const promptQuestionForSelectTargetConfigItem = () => {
    return inquirer.prompt([
        {
            type: 'autocomplete',
            name: 'systemConfigTarget',
            message: 'Select config target    ',
            source: async (answersSoFar: any, input = '') => {
                return Object.values(systemConfig).map((systemConfigItem: SystemConfigItem) => systemConfigItem.key)
            }
        }
    ])
}

interface PromptQuestion {
    type: string;
    name: string;
    choices?: Array<string | number | SystemConfigChildSelectItemData>;
    message: string;
    validate?: Function;
    default?: Boolean | number | string;
}

const buildChoices = async (configItem: SystemConfigChildNormalItem) => {
    const targetType = configItem.type || '';
    switch (targetType) {
        case 'select':
            return (configItem.data || []).map((item: SystemConfigChildSelectItemData) => item.value)
        case 'sources':
            const sourceList: SystemConfigChildSelectItemData[] = await getSourceList();
            return sourceList;
        default:
            return [];
    } 
}

const parseConfigItemQuestions = async (configItem: SystemConfigChildNormalItem, existedConfigData: StringToAny): Promise<PromptQuestion> => {
    const { required, name, key, type, tip } = configItem;
    const targetType = getQuestionInputType(type);
    const existedValue = existedConfigData[key];
    return {
        type: targetType,
        name: key,
        choices: await buildChoices(configItem),
        message: `[ ${key} ] ${tip}`,
        default: existedValue,
        validate: async (value: string | number) => {
            let hasError: Error | null = null; 
            const validatorList: Array<any> = [required ? requiredValidator : null]
            validatorList
                .filter((v: Function | null): boolean => !!v)
                .forEach((v: Function) => {
                    v(null, value, (err: Error) => {
                        if(err) hasError = err;
                    })
                })

            if(hasError) return hasError
            else return true;
        },
    }
}

export const setSystemConfig = async () => {
    const configItemKey = (await promptQuestionForSelectTargetConfigItem()).systemConfigTarget;
    const systemConfigOfTarget = systemConfig[configItemKey]
    const configType = systemConfigOfTarget.configType || '';
    const configChildItems = systemConfigOfTarget.config.filter((config: SystemConfigChildNormalItem) => !!config.cli)
    const configValueData = getValueByConfigItem(configType, configItemKey)
    const questions = await Promise.all(configChildItems.map((cItem: SystemConfigChildNormalItem) => parseConfigItemQuestions(cItem, configValueData)))
    return inquirer
        .prompt(questions)
        .then((answers: StringToAny) => setValueByConfigItem(configType, configItemKey, answers))
}

