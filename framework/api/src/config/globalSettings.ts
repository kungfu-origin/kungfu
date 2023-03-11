import fse from 'fs-extra';
import os from 'os';
import {
  OrderInputKeyEnum,
  SpaceSizeSettingEnum,
  SpaceTabSettingEnum,
} from '../typings/enums';
import { KF_CONFIG_PATH, PY_WHL_DIR } from './pathConfig';
import {
  CodeSizeSetting,
  CodeTabSetting,
  OrderInputKeySetting,
} from './tradingConfig';
import {
  languageList,
  langDefault,
} from '@kungfu-trader/kungfu-js-api/language';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { readRootPackageJsonSync } from '@kungfu-trader/kungfu-js-api/utils/fileUtils';
const { t } = VueI18n.global;
const numCPUs = os.cpus() ? os.cpus().length : 1;

export interface KfSystemConfig {
  key: string;
  name: string;
  config: KungfuApi.KfConfigItem[];
}

const __python_version_resolved = __python_version
  ? [...__python_version.split('.').slice(0, 2), 'x'].join('.')
  : '3.9.x';

const packageJson = readRootPackageJsonSync();

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
        tip: t('globalSettingConfig.select_language_desc'),
        type: 'select',
        options: languageList,
        default: langDefault,
      },
      {
        key: 'autoRestartTd',
        name: t('globalSettingConfig.auto_restart_td'),
        tip: t('globalSettingConfig.auto_restart_td_desc'),
        type: 'bool',
        default: true,
      },
      {
        key: 'bypassArchive',
        name: t('globalSettingConfig.bypass_archive'),
        tip: t('globalSettingConfig.bypass_archive_desc'),
        type: 'bool',
        default: false,
      },
    ],
  },
  {
    key: 'performance',
    name: t('globalSettingConfig.porformance'),
    config: [
      {
        key: 'rocket',
        name: t('globalSettingConfig.rocket_model'),
        tip: t('globalSettingConfig.rocket_model_desc'),
        default: false,
        disabled: numCPUs <= 4,
        type: 'bool',
      },
      {
        key: 'bypassAccounting',
        name: t('globalSettingConfig.bypass_accounting'),
        tip: t('globalSettingConfig.bypass_accounting_desc'),
        default: false,
        type: 'bool',
      },
      {
        key: 'bypassTradingData',
        name: t('globalSettingConfig.bypass_trading_data'),
        tip: t('globalSettingConfig.bypass_trading_data_desc'),
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
        tip: t('globalSettingConfig.local_python_desc', {
          py_version: __python_version_resolved,
          whl_dir_path: PY_WHL_DIR,
        }),
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
    key: 'currency',
    name: t('globalSettingConfig.currency'),
    config: [
      {
        key: 'instrumentCurrency',
        name: t('globalSettingConfig.instrument_currency'),
        tip: t('globalSettingConfig.instrument_currency_desc'),
        default: true,
        type: 'bool',
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
        min: 0,
      },
      {
        key: 'close',
        name: t('globalSettingConfig.close_threshold'),
        tip: t('globalSettingConfig.set_close_threshold'),
        default: '',
        type: 'percent',
      },

      {
        key: 'limit',
        name: t('globalSettingConfig.trade_limit'),
        tip: t('globalSettingConfig.set_trade_limit'),
        default: [],
        type: 'table',
        columns: [
          {
            key: 'instrument',
            name: t('tradeConfig.instrument_id'),
            type: 'instrument',
          },
          {
            key: 'orderInputKey',
            name: t('globalSettingConfig.order_input_key'),
            type: 'select',
            options: [
              {
                value: OrderInputKeyEnum.VOLUME,
                label: OrderInputKeySetting[OrderInputKeyEnum.VOLUME].name,
              },
              {
                value: OrderInputKeyEnum.PRICE,
                label: OrderInputKeySetting[OrderInputKeyEnum.PRICE].name,
              },
            ],
          },
          {
            key: 'limitValue',
            name: t('globalSettingConfig.limit_value'),
            default: 0,
            type: 'float',
          },
        ],
      },
      {
        key: 'assetMargin',
        name: t('globalSettingConfig.asset_margin'),
        tip: t('globalSettingConfig.show_asset_margin'),
        default: false,
        type: 'bool',
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
  ...(packageJson?.kungfuCraft?.autoUpdate?.update
    ? [
        {
          key: 'update',
          name: t('globalSettingConfig.update'),
          config: [
            {
              key: 'isCheckVersion',
              name: t('globalSettingConfig.is_check_version'),
              tip: t('globalSettingConfig.is_check_version_desc'),
              default: true,
              type: 'bool' as KungfuApi.KfConfigItemSupportedTypes,
            },
          ],
        },
      ]
    : []),
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
  return Promise.resolve(fse.writeJSONSync(KF_CONFIG_PATH, value));
};

export const riskSettingConfig: KfSystemConfig = {
  key: 'riskSetting',
  name: '风控',
  config: [
    {
      key: 'riskSetting',
      name: t('风控'),
      tip: t('风控描述'),
      default: [],
      type: 'table',
      columns: [
        {
          key: 'account_id',
          name: t('账户'),
          type: 'td',
        },
        {
          key: 'max_order_volume',
          name: t('单比最大量'),
          type: 'int',
        },
        {
          key: 'max_daily_volume',
          name: t('每日最大成交量'),
          type: 'int',
        },
        {
          key: 'self_filled_check',
          name: t('防止自成交'),
          type: 'bool',
          default: false,
        },
        {
          key: 'max_cancel_ratio',
          name: t('最大回撤率'),
          type: 'int',
        },
        {
          key: 'white_list',
          name: t('标的白名单'),
          type: 'instruments',
        },
      ],
    },
  ],
};
