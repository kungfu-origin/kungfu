import fse from 'fs-extra';
import { SpaceSizeSettingEnum, SpaceTabSettingEnum } from '../typings/enums';
import { KF_CONFIG_PATH } from './pathConfig';
import { CodeSizeSetting, CodeTabSetting } from './tradingConfig';
import {
  languageList,
  langDefault,
} from '@kungfu-trader/kungfu-app/src/language/index';
import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

export interface KfSystemConfig {
  key: string;
  name: string;
  config: KungfuApi.KfConfigItem[];
}

export const getKfGlobalSettings = (): KfSystemConfig[] => [
  {
    key: 'system',
    name: t('globalSettingConfig.system'),
    config: [
      {
        key: 'logLevel',
        name: t('globalSettingConfig.log_level'),
        tip: t('globalSettingConfig.for_all_log'),
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
      {
        key: 'language',
        name: t('globalSettingConfig.language'),
        tip: t('globalSettingConfig.select_language'),
        type: 'select',
        options: languageList,
        default: langDefault,
      },
    ],
  },
  {
    key: 'performance',
    name: t('globalSettingConfig.porformance'),
    config: [
      {
        key: 'rocket',
        name: t('globalSettingConfig.open_rocket_model'),
        tip: t('globalSettingConfig.rocket_model_desc'),
        default: false,
        type: 'bool',
      },
      {
        key: 'bypassQuote',
        name: t('globalSettingConfig.pass_quote'),
        tip: t('globalSettingConfig.pass_quote_desc'),
        default: false,
        type: 'bool',
      },
    ],
  },
  {
    key: 'strategy',
    name: t('globalSettingConfig.strategy'),
    config: [
      {
        key: 'python',
        name: t('globalSettingConfig.use_local_python'),
        tip: `${t(
          'globalSettingConfig.local_python_desc',
        )} ${__python_version}`,
        default: false,
        type: 'bool',
      },
      {
        key: 'pythonPath',
        name: t('globalSettingConfig.python_path'),
        tip: t('globalSettingConfig.python_path_desc'),
        default: '',
        type: 'file',
      },
    ],
  },
  {
    key: 'trade',
    name: t('globalSettingConfig.trade'),
    config: [
      {
        key: 'sound',
        name: t('globalSettingConfig.sound'),
        tip: t('globalSettingConfig.use_sound'),
        default: false,
        type: 'bool',
      },
      {
        key: 'fatFinger',
        name: t('globalSettingConfig.fat_finger_threshold'),
        tip: t('globalSettingConfig.set_fat_finger'),
        default: '',
        type: 'percent',
      },
      {
        key: 'close',
        name: t('globalSettingConfig.close_threshold'),
        tip: t('globalSettingConfig.set_close_threshold'),
        default: '',
        type: 'percent',
      },
    ],
  },
  {
    key: 'code',
    name: t('globalSettingConfig.code_editor'),
    config: [
      {
        key: 'tabSpaceType',
        name: t('globalSettingConfig.tab_space_type'),
        tip: t('globalSettingConfig.set_tab_space'),
        default: CodeTabSetting[SpaceTabSettingEnum.SPACES].name,
        type: 'select',
        options: [
          {
            value: SpaceTabSettingEnum.SPACES,
            label: CodeTabSetting[SpaceTabSettingEnum.SPACES].name,
          },
          {
            value: SpaceTabSettingEnum.TABS,
            label: CodeTabSetting[SpaceTabSettingEnum.TABS].name,
          },
        ],
      },
      {
        key: 'tabSpaceSize',
        name: t('globalSettingConfig.tab_space_size'),
        tip: t('globalSettingConfig.set_tab_space_size'),
        default: CodeSizeSetting[SpaceSizeSettingEnum.FOURINDENT].name,
        type: 'select',
        options: [
          {
            value: SpaceSizeSettingEnum.TWOINDENT,
            label: CodeSizeSetting[SpaceSizeSettingEnum.TWOINDENT].name,
          },
          {
            value: SpaceSizeSettingEnum.FOURINDENT,
            label: CodeSizeSetting[SpaceSizeSettingEnum.FOURINDENT].name,
          },
        ],
      },
    ],
  },
];

export const getKfGlobalSettingsValue = (): Record<
  string,
  Record<string, KungfuApi.KfConfigValue>
> => {
  return fse.readJSONSync(KF_CONFIG_PATH) as Record<
    string,
    Record<string, KungfuApi.KfConfigValue>
  >;
};

export const setKfGlobalSettingsValue = (
  value: Record<string, Record<string, KungfuApi.KfConfigValue>>,
) => {
  return fse.writeJSONSync(KF_CONFIG_PATH, value);
};

export const riskSettings: KfSystemConfig = {
  key: 'riskControl',
  name: '风控',
  config: [
    {
      key: 'riskControl',
      name: '风控',
      tip: '启用风险控制, 当持仓量超过阈值时自动平仓',
      default: [],
      type: 'table',
      columns: [
        {
          key: 'account_id',
          name: '账户',
          type: 'td',
        },
        {
          key: 'source_id',
          name: '柜台',
          type: 'str',
          primary: true,
        },
        {
          key: 'max_order_volume',
          name: '单比最大量',
          type: 'int',
        },
        {
          key: 'max_daily_volume',
          name: '每日最大成交量',
          type: 'int',
        },
        {
          key: 'self_filled_check',
          name: '是否需要自成功检测',
          type: 'bool',
          default: false,
        },
        {
          key: 'max_cancel_ratio',
          name: '最大回撤率',
          type: 'int',
        },
        {
          key: 'white_list',
          name: '标的白名单',
          type: 'instruments',
        },
      ],
    },
  ],
};
