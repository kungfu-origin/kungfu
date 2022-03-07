import inquirer from 'inquirer';
import colors from 'colors';
import {
  getKfCategoryFromString,
  getPromptQuestionsBySettings,
  parseExtDataList,
} from '../assets/methods/utils';
import { KfCategoryTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import autocompletePrompt from 'inquirer-autocomplete-prompt';
import { PathPrompt } from 'inquirer-path';
import {
  getExtConfigList,
  getKfExtensionConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';

inquirer.registerPrompt('autocomplete', autocompletePrompt);
inquirer.registerPrompt('path', PathPrompt);

export const selectAccountOrStrategy = async () => {
  const answers = await inquirer.prompt([
    {
      type: 'autocomplete',
      name: 'type',
      message: 'Select targeted md, td or strategy to add    ',
      source: async () => {
        return [
          colors.yellow('md'),
          colors.cyan('td'),
          colors.blue('strategy'),
        ];
      },
    },
  ]);

  return getKfCategoryFromString(answers.type);
};

export const addMdTdStrategy = async (type: KfCategoryTypes): Promise<void> => {
  console.log(type);

  const extConfigs = await getKfExtensionConfig();

  if (type === 'md') {
    const extDataList = getExtConfigList(extConfigs, 'md');
    const extStrList = parseExtDataList(extDataList);
    const { source } = await selectKfExtPrompt(extStrList);
    const extKey = source.split('    ')[1];
    const extConfig = extConfigs['md'][extKey];
    const settings = extConfig?.settings;

    if (settings === undefined) {
      throw new Error('Please check md extension config');
    }

    const formState = await getPromptQuestionsBySettings(settings);

    const kfConfig: KungfuApi.KfConfig = {
      category: 'md',
      group: extKey,
      name: extKey,
      value: formState,
    };
  }
};

export const selectKfExtPrompt = async (
  extStrList: string[],
): Promise<PromptAnswer> => {
  return inquirer.prompt([
    {
      type: 'autocomplete',
      name: 'source',
      message: 'Select one type of source    ',
      source: async (answersSoFar: unknown, input = '') => {
        return extStrList.filter((s: string) => s.includes(input));
      },
    },
  ]);
};
