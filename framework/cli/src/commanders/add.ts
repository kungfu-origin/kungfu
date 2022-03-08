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
  getCombineValueByPrimaryKeys,
  getExtConfigList,
  getKfExtensionConfig,
  getPrimaryKeyFromKfConfigItem,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { setKfConfig } from '@kungfu-trader/kungfu-js-api/kungfu/store';

inquirer.registerPrompt('autocomplete', autocompletePrompt);
inquirer.registerPrompt('path', PathPrompt);

export const selectMdTdStrategy = async () => {
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
    const kfLocation: KungfuApi.KfLocation = {
      category: 'md',
      group: extKey,
      name: extKey,
      mode: 'live',
    };

    return setKfConfig(
      kfLocation,
      JSON.stringify({
        ...formState,
        add_time: +new Date().getTime() * Math.pow(10, 6),
      }),
    );
  } else if (type === 'td') {
    const extDataList = getExtConfigList(extConfigs, 'td');
    const extStrList = parseExtDataList(extDataList);
    const { source } = await selectKfExtPrompt(extStrList);
    const extKey = source.split('    ')[1];
    const extConfig = extConfigs['td'][extKey];
    const settings = extConfig?.settings;

    if (settings === undefined) {
      throw new Error('Please check td extension config');
    }

    const formState = await getPromptQuestionsBySettings(settings);
    const primaryKeys = getPrimaryKeyFromKfConfigItem(settings).map(
      (item) => item.key,
    );
    const combinedValue = getCombineValueByPrimaryKeys(primaryKeys, formState);
    const kfLocation: KungfuApi.KfLocation = {
      category: 'td',
      group: extKey,
      name: combinedValue,
      mode: 'live',
    };

    return setKfConfig(
      kfLocation,
      JSON.stringify({
        ...formState,
        add_time: +new Date().getTime() * Math.pow(10, 6),
      }),
    );
  } else if (type === 'strategy') {
    const strategySettings: KungfuApi.KfConfigItem[] = [
      {
        key: 'strategy_id',
        name: '策略ID',
        type: 'str',
        primary: true,
        required: true,
        tip: '需保证该策略ID唯一',
      },
      {
        key: 'strategy_path',
        name: '策略路径',
        type: 'file',
        required: true,
      },
    ];

    const formState = await getPromptQuestionsBySettings(strategySettings);
    const primaryKeys = getPrimaryKeyFromKfConfigItem(strategySettings).map(
      (item) => item.key,
    );
    const combinedValue = getCombineValueByPrimaryKeys(primaryKeys, formState);
    const kfLocation: KungfuApi.KfLocation = {
      category: 'strategy',
      group: 'default',
      name: combinedValue,
      mode: 'live',
    };

    return setKfConfig(
      kfLocation,
      JSON.stringify({
        ...formState,
        add_time: +new Date().getTime() * Math.pow(10, 6),
      }),
    );
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
