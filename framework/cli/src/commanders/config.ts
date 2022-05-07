import {
  getKfGlobalSettings,
  getKfGlobalSettingsValue,
  KfSystemConfig,
  setKfGlobalSettingsValue,
} from '@kungfu-trader/kungfu-js-api/config/globalSettings';
import { findTargetFromArray } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import inquirer from 'inquirer';
import { getPromptQuestionsBySettings } from '../assets/methods/utils';

const kfGlobalSettings = getKfGlobalSettings();
const kfGlobalSettingsValue = getKfGlobalSettingsValue();

const promptQuestionForSelectTargetConfigItem = () => {
  return inquirer.prompt([
    {
      type: 'autocomplete',
      name: 'target',
      message: 'Select config target    ',
      source: async () => {
        return Object.values(kfGlobalSettings).map(
          (systemConfigItem: KfSystemConfig) => systemConfigItem.key,
        );
      },
    },
  ]);
};

export const setGlobalSetting = async () => {
  const targetConfigKey: { target: string } =
    await promptQuestionForSelectTargetConfigItem();

  const { target } = targetConfigKey;
  const targetConfigItem = findTargetFromArray<KfSystemConfig>(
    kfGlobalSettings,
    'key',
    target,
  );

  if (!targetConfigItem) {
    throw new Error(`Gloabl setting no ${target} option`);
  }

  const settings: KungfuApi.KfConfigItem[] = targetConfigItem.config;
  const initValue: Record<string, KungfuApi.KfConfigValue> =
    kfGlobalSettingsValue[target];

  const formState = await getPromptQuestionsBySettings(settings, initValue);
  setKfGlobalSettingsValue({
    ...kfGlobalSettingsValue,
    [target]: formState,
  });
};

export const showGlobalSetting = () => {
  console.log(getKfGlobalSettingsValue());
};
