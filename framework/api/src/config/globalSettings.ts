import fse from 'fs-extra';
import { KF_CONFIG_PATH, EXTENSION_DIR } from './pathConfig';

const isEnglish = false;

export interface KfSystemConfig {
  key: string;
  name: string;
  config: KungfuApi.KfConfigItem[];
}

export const getKfGlobalSettings = (): KfSystemConfig[] => [
  {
    key: 'system',
    name: isEnglish ? 'System' : '系统',
    config: [
      {
        key: 'extPaths',
        name: isEnglish ? 'Extension Paths' : '系统插件路径',
        tip: isEnglish
          ? 'System extensions path setting'
          : '系统插件包含柜台, 任务插件等, 如有额外的插件, 可设置对应插件路径',
        default: [EXTENSION_DIR],
        type: 'folder',
      },
      {
        key: 'logLevel',
        name: isEnglish ? 'Log level' : '全局日志级别',
        tip: isEnglish ? 'For all log' : '对系统内所有日志级别的设置',
        type: 'select',
        options: [
          { value: '-l trace', label: 'TRACE' },
          { value: '-l debug', label: 'DEBUG' },
          { value: '-l info', label: 'INFO' },
          { value: '-l warning', label: 'WARN' },
          { value: '-l error', label: 'ERROR' },
          { value: '-l critical', label: 'CRITICAL' },
        ],
        default: '-l info',
      },
    ],
  },
  {
    key: 'Performance',
    name: isEnglish ? 'Performance' : '性能',
    config: [
      {
        key: 'rocket',
        name: isEnglish ? 'Open Rocket Model' : '开启极速模式',
        tip: isEnglish
          ? 'Use CPU 100%, restart is required'
          : '开启极速模式会极大的降低系统延迟，并会使 CPU 使用效率达到100%，开启后请重启功夫交易系统',
        default: false,
        type: 'bool',
      },
      {
        key: 'bypassQuote',
        name: isEnglish ? 'Close Quote Listener' : '跳过行情计算',
        tip: isEnglish
          ? 'Close Quote Listener'
          : '在订阅一定数量（上千）支Ticker时，由行情推送触发pnl计算会导致性能问题，开启后会掉过行情计算',
        default: false,
        type: 'bool',
      },
    ],
  },
  {
    key: 'strategy',
    name: isEnglish ? 'Strategy' : '策略',
    config: [
      {
        key: 'python',
        name: isEnglish ? 'Use Local Python' : '使用本地python',
        tip: isEnglish
          ? `Pip3 install kungfu*.whl, local python require ${__python_version}`
          : `使用本地python启动策略，需要 pip3 install kungfu*.whl，本地 python3 版本需为 ${__python_version}，开启后需重启策略`,
        default: false,
        type: 'bool',
      },
      {
        key: 'pythonPath',
        name: isEnglish ? 'Select Local Python Path' : '选择本地 Python 路径',
        tip: isEnglish
          ? 'local python path is required to be selected, and kungfu*.whl should be installed in this path'
          : '功夫将会以选择的python路径运行策略，同时需要保证 kungfu*.whl 已经通过 pip安装',
        default: '',
        type: 'file',
      },
    ],
  },
];

export const getKfGlobalSettingsValue = () => {
  return fse.readJSONSync(KF_CONFIG_PATH);
};

export const setKfGlobalSettingsValue = (
  value: Record<string, Record<string, KungfuApi.KfConfigValue>>,
) => {
  return fse.writeJSONSync(KF_CONFIG_PATH, value);
};
