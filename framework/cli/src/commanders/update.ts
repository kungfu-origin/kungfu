import { getAllKfConfigOriginData } from '@kungfu-trader/kungfu-js-api/actions';
import {
  buildObjectFromKfConfigArray,
  getPromptQuestionsBySettings,
  selectTargetKfConfig,
} from '../assets/methods/utils';
import {
  getIdByKfLocation,
  getKfExtensionConfig,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { setKfConfig } from '@kungfu-trader/kungfu-js-api/kungfu/store';

export const updateMdTdStrategy = async () => {
  const kfLocation = await selectTargetKfConfig();

  if (!kfLocation) {
    throw new Error('target is illegal kfLocation');
  }

  const extConfigs = await getKfExtensionConfig();
  const { md, td, strategy } = await getAllKfConfigOriginData();

  if (kfLocation.category === 'md') {
    const targetMd = getTargetKfConfig(md, kfLocation);
    const initValue = JSON.parse(targetMd.value || '{}');
    const extConfig = extConfigs['md'][kfLocation.group];
    const settings = extConfig?.settings;

    if (settings === undefined) {
      throw new Error('Please check md extension config');
    }

    return buildPromptAndSetConfig(settings, initValue, kfLocation);
  } else if (kfLocation.category === 'td') {
    const targetTd = getTargetKfConfig(td, kfLocation);
    const initValue = JSON.parse(targetTd.value || '{}');
    const extConfig = extConfigs['td'][kfLocation.group];
    const settings = extConfig?.settings;

    if (settings === undefined) {
      throw new Error('Please check td extension config');
    }

    return buildPromptAndSetConfig(settings, initValue, kfLocation);
  } else if (kfLocation.category === 'strategy') {
    const targetStrategy = getTargetKfConfig(strategy, kfLocation);
    const initValue = JSON.parse(targetStrategy.value || '{}');
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

    return buildPromptAndSetConfig(strategySettings, initValue, kfLocation);
  }
};

function getTargetKfConfig(
  kfConfigs: KungfuApi.KfConfig[],
  kfLocation: KungfuApi.KfLocation,
): KungfuApi.KfConfig | null {
  const kfConfigMap = buildObjectFromKfConfigArray(kfConfigs);
  return kfConfigMap[getIdByKfLocation(kfLocation)] || null;
}

async function buildPromptAndSetConfig(
  settings: KungfuApi.KfConfigItem[],
  initValue: Record<string, KungfuApi.KfConfigValue>,
  kfLocation: KungfuApi.KfLocation,
): Promise<void> {
  const formState = await getPromptQuestionsBySettings(settings, initValue);

  return setKfConfig(
    kfLocation,
    JSON.stringify({
      ...formState,
      add_time: +new Date().getTime() * Math.pow(10, 6),
    }),
  );
}
