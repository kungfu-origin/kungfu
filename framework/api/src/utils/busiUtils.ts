import path from 'path';
import dayjs from 'dayjs';
import fse, { Stats } from 'fs-extra';
import log4js from 'log4js';
import glob from 'glob';
import os from 'os';
import {
  buildProcessLogPath,
  EXTENSION_DIRS,
  KF_HOME,
  KF_RUNTIME_DIR,
} from '../config/pathConfig';
import {
  InstrumentType,
  KfCategory,
  AppStateStatus,
  Pm2ProcessStatus,
  Side,
  Offset,
  Direction,
  OrderStatus,
  HedgeFlag,
  PriceType,
  TimeCondition,
  VolumeCondition,
  ExchangeIds,
  FutureArbitrageCodes,
  CommissionMode,
  StrategyExtType,
  UnderweightType,
  ShotableInstrumentTypes,
  T0InstrumentTypes,
  T0ExchangeIds,
  PriceLevel,
  InstrumentMinOrderVolume,
} from '../config/tradingConfig';
import {
  KfCategoryEnum,
  DirectionEnum,
  OrderStatusEnum,
  LedgerCategoryEnum,
  InstrumentTypeEnum,
  SideEnum,
  OffsetEnum,
  HedgeFlagEnum,
  InstrumentTypes,
  KfCategoryTypes,
  LedgerCategoryTypes,
  ProcessStatusTypes,
  BrokerStateStatusTypes,
  PriceTypeEnum,
  TimeConditionEnum,
  VolumeConditionEnum,
  BrokerStateStatusEnum,
  StrategyExtTypes,
  CommissionModeEnum,
  StrategyStateStatusTypes,
  StrategyStateStatusEnum,
  UnderweightEnum,
  PriceLevelEnum,
} from '../typings/enums';
import {
  graceDeleteProcess,
  Pm2ProcessStatusData,
  Pm2ProcessStatusDetail,
  Pm2ProcessStatusDetailData,
  startCacheD,
  startExtDaemon,
  startLedger,
  startMaster,
  startMd,
  startStrategy,
  startTd,
} from './processUtils';
import { Proc } from 'pm2';
import {
  listDir,
  readRootPackageJsonSync,
  removeTargetFilesInFolder,
} from './fileUtils';
import minimist from 'minimist';
import VueI18n, { useLanguage } from '../language';
import { unlinkSync } from 'fs-extra';
import { T0T1Config } from '../typings/global';
import { getKfGlobalSettingsValue } from '../config/globalSettings';
const { t } = VueI18n.global;
interface SourceAccountId {
  source: string;
  id: string;
}

declare global {
  interface String {
    toAccountId(): string;
    parseSourceAccountId(): SourceAccountId;
    toSourceName(): string;
    toStrategyId(): string;
    toKfCategory(): string;
    toKfGroup(): string;
    toKfName(): string;
  }

  interface Array<T> {
    removeRepeat(): Array<T>;
    kfForEach(cb: <T>(t: T, index: number) => void): void;
    kfReverseForEach(cb: <T>(t: T, index: number) => void): void;
    kfForEachAsync(cb: <T>(t: T, index: number) => void): void;
  }
}

//for td processId
String.prototype.toAccountId = function (): string {
  if (this.indexOf('_') === -1) return this.toString();
  if (this.split('_').length !== 3) return this.toString();
  return this.split('_').slice(1).join('_');
};

//for md processId
String.prototype.toSourceName = function (): string {
  if (this.indexOf('_') === -1) return this.toString();
  if (this.split('_').length !== 2) return this.toString();
  return this.split('_')[1];
};

//for strategy processId
String.prototype.toStrategyId = function (): string {
  if (this.indexOf('_') === -1) return this.toString();
  if (this.split('_').length !== 2) return this.toString();
  return this.split('_')[1];
};

String.prototype.toKfCategory = function (): string {
  if (this.indexOf('_') === -1) return this.toString();
  if (this.split('_').length !== 3) return this.toString();
  return this.split('_')[0];
};

String.prototype.toKfGroup = function (): string {
  if (this.indexOf('_') === -1) return this.toString();
  if (this.split('_').length !== 3) return this.toString();
  return this.split('_')[1];
};

String.prototype.toKfName = function (): string {
  if (this.indexOf('_') === -1) return this.toString();
  if (this.split('_').length !== 3) return this.toString();
  return this.split('_')[2];
};

String.prototype.parseSourceAccountId = function (): SourceAccountId {
  const parseList = this.toString().split('_');
  //没有 "_"
  if (parseList.length !== 2) {
    throw new Error(`${this} accountId format is wrong！`);
  } else {
    return {
      source: parseList[0],
      id: parseList[1],
    };
  }
};

Array.prototype.removeRepeat = function () {
  return Array.from(new Set(this));
};

Array.prototype.kfForEach = function (cb) {
  if (!cb) return;
  const self = this;
  const len = self.length;
  let i = 0;

  while (i < len) {
    cb.call(self, self[i], i);
    i++;
  }
};

Array.prototype.kfReverseForEach = function (cb) {
  if (!cb) return;
  const self = this;
  let i = self.length;
  while (i--) {
    cb.call(self, self[i], i);
  }
};

Array.prototype.kfForEachAsync = function (cb) {
  if (!cb) return;
  const self = this;
  const len = self.length;
  return new Promise((resolve) => {
    setImmediateIter(self, 0, len, cb, () => {
      resolve(true);
    });
  });
};

function setImmediateIter<T>(
  list: Array<T>,
  i: number,
  len: number,
  cb: (item: T, index: number) => void,
  fcb: AnyFunction,
) {
  if (i < len) {
    setImmediate(() => {
      cb(list[i], i);
      setImmediateIter(list, ++i, len, cb, fcb);
    });
  } else {
    fcb();
  }
}

log4js.configure({
  appenders: {
    app: {
      type: 'file',
      filename: buildProcessLogPath('app'),
    },
  },
  categories: { default: { appenders: ['app'], level: 'info' } },
});

export const logger = log4js.getLogger('app');

export const kfLogger = {
  info: (...args: Array<unknown>) => {
    if (
      process.env.NODE_ENV === 'development' ||
      process.env.APP_TYPE !== 'cli'
    ) {
      console.log('<KF_INFO>', ...args);
    }
    logger.info('<KF_INFO>', args.join(' '));
  },

  warn: (...args: Array<unknown>) => {
    if (
      process.env.NODE_ENV === 'development' ||
      process.env.APP_TYPE !== 'cli'
    ) {
      console.warn('<KF_WARN>', ...args);
    }
    logger.warn('<KF_WARN>', args.join(' '));
  },

  error: (...args: Array<unknown>) => {
    if (
      process.env.NODE_ENV === 'development' ||
      process.env.APP_TYPE !== 'cli'
    ) {
      console.error('<KF_ERROR>', ...args);
    }
    logger.error('<KF_ERROR>', args.join(' '));
  },
};

export const dealSpaceInPath = (pathname: string) => {
  const normalizePath = path.normalize(pathname);
  return normalizePath.replace(/ /g, ' ');
};

export const setTimerPromiseTask = (fn: AnyPromiseFunction, interval = 500) => {
  let taskTimer: number | undefined = undefined;
  let clear = false;
  function timerPromiseTask(fn: AnyPromiseFunction, interval = 500) {
    if (taskTimer)
      globalThis.clearTimeout(taskTimer as unknown as NodeJS.Timeout);
    fn().finally(() => {
      if (clear) {
        if (taskTimer)
          globalThis.clearTimeout(taskTimer as unknown as NodeJS.Timeout);
        return;
      }
      taskTimer = +globalThis.setTimeout(() => {
        timerPromiseTask(fn, interval);
      }, interval);
    });
  }
  timerPromiseTask(fn, interval);
  return {
    clearLoop: function () {
      clear = true;
      if (taskTimer != null)
        globalThis.clearTimeout(taskTimer as unknown as NodeJS.Timeout);
    },
  };
};

export const loopToRunProcess = async <T>(
  promiseFunc: Array<() => Promise<T>>,
  interval = 100,
) => {
  let i = 0;
  const len = promiseFunc.length;
  const resList: (T | Error)[] = [];
  for (i = 0; i < len; i++) {
    const pFunc = promiseFunc[i];
    try {
      const res: T = await pFunc();
      resList.push(res);
    } catch (err: unknown) {
      resList.push(err as Error);
    }

    await delayMilliSeconds(interval);
  }
  return resList;
};

export const delayMilliSeconds = (miliSeconds: number): Promise<void> => {
  return new Promise((resolve) => {
    const timer = setTimeout(() => {
      resolve();
      clearTimeout(timer);
    }, miliSeconds);
  });
};

export const findTargetFromArray = <T>(
  list: Array<T>,
  targetKey: string,
  targetValue: string | number | boolean,
) => {
  const targetList = list.filter(
    (item) => (item || {})[targetKey] === targetValue,
  );
  if (targetList && targetList.length) {
    return targetList[0];
  }
  return null;
};

export const buildObjectFromArray = <T>(
  list: Array<T>,
  targetKey: number | string,
  targetValueKey?: number | string,
): Record<string | number, T | T[keyof T] | undefined> => {
  return list.reduce((item1, item2) => {
    const key: number | string | symbol = (item2 || {})[targetKey] || '';
    if (key !== '' && key !== undefined) {
      if (targetValueKey === undefined) {
        item1[key] = item2;
      } else {
        item1[key] = (item2 || {})[targetValueKey];
      }
    }
    return item1;
  }, {} as Record<string | number, T | T[keyof T] | undefined>);
};

export const getInstrumentTypeData = (
  instrumentType: InstrumentTypes,
): KungfuApi.KfTradeValueCommonData => {
  return InstrumentType[
    (InstrumentTypeEnum[instrumentType] as InstrumentTypeEnum) ||
      InstrumentTypeEnum.unknown
  ];
};

export const getStrategyExtTypeData = (
  strategyExtType: StrategyExtTypes,
): KungfuApi.KfTradeValueCommonData => {
  return StrategyExtType[strategyExtType || 'unknown'];
};

const getChildFileStat = async (
  dirname: string,
): Promise<Array<{ childFilePath: string; stat: Stats }>> => {
  if (!(await fse.pathExists(dirname))) {
    return [];
  }

  const cDirs = await fse.readdir(dirname);
  const statsDatas: Array<{ childFilePath: string; stat: Stats }> =
    await Promise.all(
      cDirs.map((cDir: string) => {
        const childFilePath = path.join(dirname, cDir);
        return fse.stat(childFilePath).then((stat: Stats) => {
          return {
            childFilePath,
            stat,
          };
        });
      }),
    );

  return statsDatas;
};

export const flattenExtensionModuleDirs = async (
  extensionDirs: string[],
): Promise<string[]> => {
  let extensionModuleDirs: string[] = [];
  const statsList = await Promise.all(
    extensionDirs.map((dirname: string) => {
      return getChildFileStat(dirname);
    }),
  );

  let i = 0;
  const len = statsList.length;
  for (i = 0; i < len; i++) {
    const statsDatas = statsList[i];
    for (let r = 0; r < statsDatas.length; r++) {
      const statsData = statsDatas[r];
      const { childFilePath, stat } = statsData;
      if (stat.isDirectory()) {
        if (
          process.env.NODE_ENV === 'production' ||
          childFilePath.includes('dist')
        ) {
          if (fse.pathExistsSync(path.join(childFilePath, 'package.json'))) {
            extensionModuleDirs.push(childFilePath);
          } else {
            const extModules = await flattenExtensionModuleDirs([
              childFilePath,
            ]);
            extensionModuleDirs = extensionModuleDirs.concat(extModules);
          }
        } else {
          const extModules = await flattenExtensionModuleDirs([
            path.join(childFilePath, 'dist'),
          ]);
          extensionModuleDirs = extensionModuleDirs.concat(extModules);
        }
      }
    }
  }

  return extensionModuleDirs;
};

const getKfExtConfigList = async (): Promise<KungfuApi.KfExtOriginConfig[]> => {
  const extModuleDirs = await flattenExtensionModuleDirs(EXTENSION_DIRS);
  const packageJSONPaths = extModuleDirs.map((item) =>
    path.join(item, 'package.json'),
  );
  return await Promise.all(
    packageJSONPaths.map((item) => {
      return fse.readJSON(item).then((jsonConfig) => {
        return {
          ...(jsonConfig.kungfuConfig || {}),
          extPath: path.dirname(item),
        };
      });
    }),
  ).then((configList: KungfuApi.KfExtOriginConfig[]) => {
    return configList.filter(
      (
        config: KungfuApi.KfExtOriginConfig,
      ): config is KungfuApi.KfExtOriginConfig => !!config,
    );
  });
};

const resolveTypesInExtConfig = (
  category: KfCategoryTypes,
  types:
    | InstrumentTypes
    | InstrumentTypes[]
    | StrategyExtTypes
    | StrategyExtTypes[],
): InstrumentTypes[] | StrategyExtTypes[] => {
  if (typeof types === 'string') {
    const typesResolved = [
      types.toLowerCase() as InstrumentTypes | StrategyExtTypes,
    ];
    return isTdMd(category)
      ? (typesResolved as InstrumentTypes[])
      : (typesResolved as StrategyExtTypes[]);
  }

  if (!types.length) {
    return ['unknown'];
  }

  const typesResolved = types.map((type) => type.toLowerCase());
  return isTdMd(category)
    ? (typesResolved as InstrumentTypes[])
    : (typesResolved as StrategyExtTypes[]);
};

const getKfExtensionConfigByCategory = (
  extConfigs: KungfuApi.KfExtOriginConfig[],
): KungfuApi.KfExtConfigs => {
  return extConfigs
    .filter((item) => !!item.config)
    .reduce((configByCategory, extConfig: KungfuApi.KfExtOriginConfig) => {
      const extKey = extConfig.key;
      const extName = extConfig.name;
      const extPath = extConfig.extPath;
      (Object.keys(extConfig['config'] || {}) as KfCategoryTypes[]).forEach(
        (category: KfCategoryTypes) => {
          const configOfCategory = (extConfig['config'] || {})[category];
          configByCategory[category] = {
            ...(configByCategory[category] || {}),
            [extKey]: {
              name: extName,
              extPath,
              category,
              key: extKey,
              type: resolveTypesInExtConfig(
                category,
                configOfCategory?.type || [],
              ),
              settings: configOfCategory?.settings || [],
            },
          };
        },
      );
      return configByCategory;
    }, {} as KungfuApi.KfExtConfigs);
};

const getKfUIExtensionConfigByExtKey = (
  extConfigs: KungfuApi.KfExtOriginConfig[],
): KungfuApi.KfUIExtConfigs => {
  return extConfigs
    .filter((item) => !!item.ui_config)
    .reduce((configByExtraKey, extConfig) => {
      const extKey = extConfig.key;
      const extName = extConfig.name;
      const extPath = extConfig.extPath;
      const uiConfig = extConfig['ui_config'];
      const position = uiConfig?.position || '';
      const exhibit = uiConfig?.exhibit || ({} as KungfuApi.KfExhibitConfig);
      const components = uiConfig?.components || null;
      const daemon = uiConfig?.daemon || ({} as Record<string, string>);
      const script = uiConfig?.script || '';

      configByExtraKey[extKey] = {
        name: extName,
        extPath,
        position,
        exhibit,
        components,
        daemon,
        script,
      };
      return configByExtraKey;
    }, {} as KungfuApi.KfUIExtConfigs);
};

const getKfCliExtensionConfigByExtKey = (
  extConfigs: KungfuApi.KfExtOriginConfig[],
): KungfuApi.KfCliExtConfigs => {
  return extConfigs
    .filter((item) => !!item.cli_config)
    .reduce((configByExtraKey, extConfig) => {
      const extKey = extConfig.key;
      const extName = extConfig.name;
      const extPath = extConfig.extPath;
      const cliConfig = extConfig['cli_config'];
      const exhibit = cliConfig?.exhibit || ({} as KungfuApi.KfExhibitConfig);
      const components = cliConfig?.components || null;
      const daemon = cliConfig?.daemon || ({} as Record<string, string>);
      const script = cliConfig?.script || '';

      configByExtraKey[extKey] = {
        name: extName,
        extPath,
        exhibit,
        components,
        daemon,
        script,
      };
      return configByExtraKey;
    }, {} as KungfuApi.KfCliExtConfigs);
};

export const getKfExtensionConfig =
  async (): Promise<KungfuApi.KfExtConfigs> => {
    const kfExtConfigList = await getKfExtConfigList();
    return getKfExtensionConfigByCategory(kfExtConfigList);
  };

export const getKfUIExtensionConfig =
  async (): Promise<KungfuApi.KfUIExtConfigs> => {
    const kfExtConfigList = await getKfExtConfigList();
    return getKfUIExtensionConfigByExtKey(kfExtConfigList);
  };

export const getKfCliExtensionConfig =
  async (): Promise<KungfuApi.KfCliExtConfigs> => {
    const kfExtConfigList = await getKfExtConfigList();
    return getKfCliExtensionConfigByExtKey(kfExtConfigList);
  };

export const getExhibitConfig =
  async (): Promise<KungfuApi.KfExhibitConfigs> => {
    const KfExtConfig: KungfuApi.KfUIExtConfigs =
      await getKfUIExtensionConfig();
    return Object.keys(KfExtConfig).reduce((extensionData, key) => {
      const exhibitData: KungfuApi.KfExhibitConfig = KfExtConfig[key]?.exhibit;
      extensionData[key] = {
        type: exhibitData.type || '',
        config: exhibitData.config || [],
      };
      return extensionData;
    }, {});
  };

export const getKfExtensionLanguage = async () => {
  const kfExtConfigList = await getKfExtConfigList();

  return kfExtConfigList.reduce((languageMap, config) => {
    if ('language' in config) {
      const defaultLangData: KungfuApi.KfExtOriginConfig['language'] = {
        'zh-CN': {},
        'en-US': {},
      };
      const langData =
        typeof config.language === 'object' ? config.language : defaultLangData;

      const extNames = {
        'zh-CN': langData['zh-CN'][config.key] ?? config.name,
        'en-US':
          langData['en-US'][config.key] ??
          (config.key[0].toUpperCase() + config.key.slice(1)).replace(
            /(?<!^)([A-Z])(?![A-Z])/g,
            ' $1',
          ),
      };

      Object.keys(langData).forEach((langName) => {
        languageMap[langName] = {
          ...(languageMap[langName] || {}),
          [config.key]: langData[langName],
          [config.name]: extNames[langName] ?? config.name,
        };
      });
    }
    return languageMap;
  }, {} as KungfuApi.KfExtLanguages);
};

export const getAvailDaemonList = async (): Promise<
  KungfuApi.KfDaemonLocation[]
> => {
  const kfExtConfig: KungfuApi.KfUIExtConfigs = await getKfUIExtensionConfig();
  return Object.values(kfExtConfig || ({} as KungfuApi.KfUIExtConfigs))
    .filter((item) => Object.keys(item).length)
    .reduce((daemonList, item) => {
      daemonList = [
        ...daemonList,
        ...Object.keys(item.daemon).map((name) => ({
          category: 'daemon',
          group: 'ext',
          name,
          mode: 'live',
          cwd: item.extPath,
          script: item.daemon[name],
        })),
      ];
      return daemonList;
    }, [] as KungfuApi.KfDaemonLocation[]);
};

export const getAvailCliDaemonList = async (): Promise<
  KungfuApi.KfDaemonLocation[]
> => {
  const kfExtConfig: KungfuApi.KfCliExtConfigs =
    await getKfCliExtensionConfig();
  return Object.values(kfExtConfig || ({} as KungfuApi.KfCliExtConfigs))
    .filter((item) => Object.keys(item).length)
    .reduce((daemonList, item) => {
      daemonList = [
        ...daemonList,
        ...Object.keys(item.daemon).map((name) => ({
          category: 'daemon',
          group: 'ext',
          name,
          mode: 'live',
          cwd: item.extPath,
          script: item.daemon[name],
        })),
      ];
      return daemonList;
    }, [] as KungfuApi.KfDaemonLocation[]);
};

export const getAvailScripts = async (): Promise<string[]> => {
  const kfExtConfig: KungfuApi.KfUIExtConfigs = await getKfUIExtensionConfig();
  return Object.values(kfExtConfig || ({} as KungfuApi.KfUIExtConfigs))
    .filter((item) => Object.keys(item).length && item.script)
    .map((item) => path.resolve(item.extPath, item.script));
};

export const isTdMd = (category: KfCategoryTypes) => {
  if (category === 'td' || category === 'md') {
    return true;
  }

  return false;
};

export const isTdMdStrategy = (category: KfCategoryTypes) => {
  if (category === 'td' || category === 'md' || category === 'strategy') {
    return true;
  }

  return false;
};

export const buildExtTypeMap = (
  extConfigs: KungfuApi.KfExtConfigs,
  category: KfCategoryTypes,
): Record<string, InstrumentTypes | StrategyExtTypes> => {
  const extTypeMap: Record<string, InstrumentTypes | StrategyExtTypes> = {};
  const targetCategoryConfig: Record<string, KungfuApi.KfExtConfig> =
    extConfigs[category] || {};

  Object.keys(targetCategoryConfig).forEach((extKey: string) => {
    const configInKfExtConfig = targetCategoryConfig[extKey];
    const types = resolveTypesInExtConfig(
      category,
      configInKfExtConfig?.type || [],
    );

    if (!types.length) {
      extTypeMap[extKey] = 'unknown';
      return;
    }

    const primaryType = isTdMd(category)
      ? (types as InstrumentTypes[]).sort(
          (type1: InstrumentTypes, type2: InstrumentTypes) => {
            const level1 =
              (
                InstrumentType[
                  InstrumentTypeEnum[type1] || InstrumentTypeEnum.unknown
                ] || {}
              ).level || 0;
            const level2 =
              (
                InstrumentType[
                  InstrumentTypeEnum[type2] || InstrumentTypeEnum.unknown
                ] || {}
              ).level || 0;
            return level2 - level1;
          },
        )[0]
      : (types as StrategyExtTypes[]).sort(
          (type1: StrategyExtTypes, type2: StrategyExtTypes) => {
            const level1 = (StrategyExtType[type1] || {}).level || 0;
            const level2 = (StrategyExtType[type2] || {}).level || 0;
            return level2 - level1;
          },
        )[0];

    extTypeMap[extKey] = primaryType;
  });

  return extTypeMap;
};

export const getExtConfigList = (
  extConfigs: KungfuApi.KfExtConfigs,
  category: KfCategoryTypes,
): KungfuApi.KfExtConfig[] => {
  return Object.values(extConfigs[category] || {});
};

export const statTime = (name: string) => {
  if (process.env.NODE_ENV !== 'production') {
    console.time(name);
  }
};

export const statTimeEnd = (name: string) => {
  if (process.env.NODE_ENV !== 'production') {
    console.timeEnd(name);
  }
};

export const hidePasswordByLogger = (config: string) => {
  const configCopy = JSON.parse(config);
  Object.keys(configCopy || {}).forEach((key: string) => {
    if (key.includes('password')) {
      configCopy[key] = '******';
    }
  });
  return JSON.stringify(configCopy);
};

export const dealDateDayOrMonth = (val: number) => {
  return val < 10 ? `0${val}` : `${val}`;
};

export const removeArchiveBeforeToday = (
  targetFolder: string,
): Promise<void> => {
  const today = dayjs();
  const year = today.year();
  const month = today.month() + 1;
  const day = today.date();
  const todayArchive = `KFA-${year}-${dealDateDayOrMonth(
    month,
  )}-${dealDateDayOrMonth(day)}.zip`;
  return removeTargetFilesInFolder(targetFolder, ['.zip'], [todayArchive]);
};

export const removeJournal = (targetFolder: string): Promise<void> => {
  return removeTargetFilesInFolder(targetFolder, ['.journal']);
};

export const removeDB = (targetFolder: string): Promise<void> => {
  return removeTargetFilesInFolder(targetFolder, ['.db'], ['config.db']);
};

export const getProcessIdByKfLocation = (
  kfLocation: KungfuApi.KfLocation,
): string => {
  if (kfLocation.category === 'td') {
    return `${kfLocation.category}_${kfLocation.group}_${kfLocation.name}`;
  } else if (kfLocation.category === 'md') {
    return `${kfLocation.category}_${kfLocation.group}`;
  } else if (kfLocation.category === 'strategy') {
    if (kfLocation.group === 'default') {
      return `${kfLocation.category}_${kfLocation.name}`;
    } else {
      return `${kfLocation.category}_${kfLocation.group}_${kfLocation.name}`;
    }
  } else if (kfLocation.category === 'system') {
    return kfLocation.name;
  } else {
    return `${kfLocation.category}_${kfLocation.group}_${kfLocation.name}`;
  }
};

export const getIdByKfLocation = (kfLocation: KungfuApi.KfLocation): string => {
  if (kfLocation.category === 'td') {
    return `${kfLocation.group}_${kfLocation.name}`;
  } else if (kfLocation.category === 'md') {
    return `${kfLocation.group}`;
  } else if (kfLocation.category === 'strategy') {
    if (kfLocation.group === 'default') {
      return `${kfLocation.name}`;
    } else {
      return `${kfLocation.group}_${kfLocation.name}`;
    }
  } else if (kfLocation.category === 'system') {
    return `${kfLocation.group}_${kfLocation.name}`;
  } else {
    return `${kfLocation.group}_${kfLocation.name}`;
  }
};

export const getMdTdKfLocationByProcessId = (
  processId: string,
): KungfuApi.KfLocation | null => {
  if (processId.indexOf('td_') === 0) {
    if (processId.split('_').length === 3) {
      const [category, group, name] = processId.split('_');
      return {
        category: category as KfCategoryTypes,
        group,
        name,
        mode: 'live',
      };
    }
  } else if (processId.indexOf('md_') === 0) {
    if (processId.split('_').length === 2) {
      const [category, group] = processId.split('_');
      return {
        category: category as KfCategoryTypes,
        group,
        name: group,
        mode: 'live',
      };
    }
  }

  return null;
};

export const getStrategyKfLocationByProcessId = (
  processId: string,
): KungfuApi.KfLocation | null => {
  if (processId.indexOf('strategy_') === 0) {
    const splits = processId.split('_');

    if (splits.length === 3) {
      const [category, group, name] = processId.split('_');
      return {
        category: category as KfCategoryTypes,
        group,
        name,
        mode: 'live',
      };
    } else if (splits.length === 2) {
      const [category, name] = processId.split('_');
      return {
        category: category as KfCategoryTypes,
        group: 'default',
        name,
        mode: 'live',
      };
    }
  }

  return null;
};

export const getDaemonKfLocationByProcessId = (
  processId: string,
): KungfuApi.KfLocation | null => {
  if (processId.indexOf('daemon_') === 0) {
    const arr = processId.split('_');
    if (arr.length < 3) return null;

    return {
      category: arr[0],
      group: arr[1],
      name: arr[2],
      mode: 'live',
    };
  }
  return null;
};

const getSystemKfLocationProcessId = (processId: string) => {
  if (!processId) return null;
  if (processId === 'master') {
    return {
      category: 'system',
      group: processId,
      name: processId,
      mode: 'live',
    };
  } else if (
    ['ledger', 'archive', 'cached', 'dzxy'].indexOf(processId) !== -1
  ) {
    return {
      category: 'system',
      group: 'service',
      name: processId,
      mode: 'live',
    };
  }

  return null;
};

export const getKfLocationByProcessId = (
  processId: string,
): KungfuApi.KfLocation | null => {
  if (processId.indexOf('td_') === 0 || processId.indexOf('md_') === 0) {
    return getMdTdKfLocationByProcessId(processId);
  } else if (processId.indexOf('strategy_') === 0) {
    return getStrategyKfLocationByProcessId(processId);
  } else if (processId.indexOf('daemon_') === 0) {
    return getDaemonKfLocationByProcessId(processId);
  } else if (processId.indexOf('_') === -1) {
    return getSystemKfLocationProcessId(processId);
  }

  return null;
};

export const isTdStrategyCategory = (category: string): boolean => {
  if (category !== 'td') {
    if (category !== 'strategy') {
      return false;
    }
  }

  return true;
};

export const getStateStatusData = (
  name: ProcessStatusTypes | undefined,
): KungfuApi.KfTradeValueCommonData | undefined => {
  return name === undefined ? undefined : AppStateStatus[name];
};

export const getIfProcessRunning = (
  processStatusData: Pm2ProcessStatusData,
  processId: string,
): boolean => {
  const statusName = processStatusData[processId] || '';
  if (statusName) {
    if ((Pm2ProcessStatus[statusName].level || 0) > 0) {
      return true;
    }
  }

  return false;
};

export const getIfProcessDeleted = (
  processStatusData: Pm2ProcessStatusData,
  processId: string,
): boolean => {
  return processStatusData[processId] === undefined;
};

export const getIfProcessStopping = (
  processStatusData: Pm2ProcessStatusData,
  processId: string,
) => {
  const statusName = processStatusData[processId] || '';
  if (statusName) {
    if (Pm2ProcessStatus[statusName].level === 1) {
      return true;
    }
  }

  return false;
};

export const getAppStateStatusName = (
  kfConfig: KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
  appStates: Record<string, BrokerStateStatusTypes>,
): ProcessStatusTypes | undefined => {
  const processId = getProcessIdByKfLocation(kfConfig);

  if (!processStatusData[processId]) {
    return undefined;
  }

  if (!getIfProcessRunning(processStatusData, processId)) {
    return undefined;
  }

  if (appStates[processId]) {
    return appStates[processId];
  }

  const processStatus = processStatusData[processId];
  return processStatus;
};

export const getStrategyStateStatusName = (
  kfConfig: KungfuApi.KfLocation,
  processStatusData: Pm2ProcessStatusData,
  strategyStates: Record<string, KungfuApi.StrategyStateData>,
): ProcessStatusTypes | undefined => {
  const processId = getProcessIdByKfLocation(kfConfig);

  if (!processStatusData[processId]) {
    return undefined;
  }

  if (!getIfProcessRunning(processStatusData, processId)) {
    return undefined;
  }

  if (strategyStates[processId]) {
    return strategyStates[processId].state;
  }

  const processStatus = processStatusData[processId];
  return processStatus;
};

export const getPropertyFromProcessStatusDetailDataByKfLocation = (
  processStatusDetailData: Pm2ProcessStatusDetailData,
  kfLocation: KungfuApi.KfLocation,
): {
  status: ProcessStatusTypes | undefined;
  cpu: number;
  memory: string;
} => {
  const processStatusDetail: Pm2ProcessStatusDetail =
    processStatusDetailData[getProcessIdByKfLocation(kfLocation)] ||
    ({} as Pm2ProcessStatusDetail);
  const status = processStatusDetail.status;
  const monit = processStatusDetail.monit || {};

  return {
    status,
    cpu: monit.cpu || 0,
    memory: Number((monit.memory || 0) / (1024 * 1024)).toFixed(2),
  };
};

export class KfNumList<T> {
  list: T[];
  limit: number;

  constructor(limit: number) {
    this.list = [];
    this.limit = limit;
  }

  insert(item: T) {
    if (this.list.length >= this.limit) this.list.shift();
    this.list.push(item);
  }
}

export const debounce = (fn: AnyFunction, delay = 300, immediate = false) => {
  let timeout: number;
  return (...args: Array<unknown>) => {
    if (immediate && !timeout) {
      fn(...args);
    }
    clearTimeout(timeout);

    timeout = +setTimeout(() => {
      fn(...args);
    }, delay);
  };
};

export const getConfigValue = (kfConfig: KungfuApi.KfConfig) => {
  return JSON.parse(kfConfig.value || '{}');
};

export const buildIdByKeysFromKfConfigSettings = (
  kfConfigState: Record<string, KungfuApi.KfConfigValue>,
  keys: string[],
) => {
  return keys
    .map((key) => kfConfigState[key])
    .filter((value) => value !== undefined)
    .join('_');
};

const startProcessByKfLocation = async (
  kfLocation: KungfuApi.DerivedKfLocation,
  force?: boolean,
) => {
  try {
    await globalThis.HookKeeper.getHooks().prestart.trigger(kfLocation);
  } catch (err) {
    console.error(err);
  }

  const isForce = force ?? true;

  switch (kfLocation.category) {
    case 'system':
      if (kfLocation.name === 'master') {
        return startMaster(isForce);
      } else if (kfLocation.name === 'ledger') {
        return startLedger(isForce);
      } else if (kfLocation.name === 'cached') {
        return startCacheD(isForce);
      }
      break;
    case 'td':
      return startTd(getIdByKfLocation(kfLocation), kfLocation);
    case 'md':
      return startMd(getIdByKfLocation(kfLocation), kfLocation);
    case 'strategy':
      const strategyPath =
        JSON.parse((kfLocation as KungfuApi.KfConfig)?.value || '{}')
          .strategy_path || '';
      if (!strategyPath) {
        throw new Error('Start Stratgy without strategy_path');
      }
      return startStrategy(getIdByKfLocation(kfLocation), strategyPath);
    case 'daemon':
      return startExtDaemon(
        getProcessIdByKfLocation(kfLocation),
        kfLocation['cwd'] || '',
        kfLocation['script'] || '',
      );
    default:
      return Promise.resolve();
  }
};

export const switchKfLocation = (
  watcher: KungfuApi.Watcher | null,
  kfLocation: KungfuApi.DerivedKfLocation,
  targetStatus: boolean,
  force?: boolean,
): Promise<void | Proc> => {
  if (!watcher) return Promise.reject(new Error('Watcher is NULL'));

  if (!targetStatus) {
    return graceDeleteProcess(watcher, kfLocation);
  }

  return startProcessByKfLocation(kfLocation, force);
};

export const isShotable = (instrumentType: InstrumentTypeEnum): boolean => {
  return instrumentType
    ? ShotableInstrumentTypes.includes(instrumentType)
    : false;
};

export const getPriceTypeConfig = (): Record<
  PriceTypeEnum,
  KungfuApi.KfTradeValueCommonData
> => {
  const rootPackageJson = readRootPackageJsonSync();
  const priceTypeConfig =
    rootPackageJson?.appConfig?.makeOrder?.priceTypeFilter ||
    ({} as Record<string, boolean>);
  const unsupportedPriceTypes = Object.keys(priceTypeConfig).filter((key) => {
    if (priceTypeConfig[key] === false && PriceTypeEnum[key] !== undefined) {
      return true;
    }
    return false;
  });

  return Object.keys(PriceTypeEnum)
    .filter((key) => Number.isNaN(+key))
    .filter((priceType) => !unsupportedPriceTypes.includes(priceType))
    .map((priceType) => PriceTypeEnum[priceType])
    .reduce((pre, enumValue: PriceTypeEnum) => {
      return { ...pre, ...{ [enumValue]: PriceType[enumValue] } };
    }, {});
};

export const getOffsetConfig = (): Record<
  PriceTypeEnum,
  KungfuApi.KfTradeValueCommonData
> => {
  const rootPackageJson = readRootPackageJsonSync();
  const offsetConfig =
    rootPackageJson?.appConfig?.makeOrder?.offsetFilter ||
    ({} as Record<string, boolean>);
  const unsupportedOffset = Object.keys(offsetConfig).filter((key) => {
    if (offsetConfig[key] === false && OffsetEnum[key] !== undefined) {
      return true;
    }
    return false;
  });

  return Object.keys(OffsetEnum)
    .filter((key) => Number.isNaN(+key))
    .filter((offset) => !unsupportedOffset.includes(offset))
    .map((offset) => OffsetEnum[offset])
    .reduce((pre, enumValue: OffsetEnum) => {
      return { ...pre, ...{ [enumValue]: Offset[enumValue] } };
    }, {});
};

export const getAbleHedgeFlag = (): boolean => {
  const rootPackageJson = readRootPackageJsonSync();
  const ableHedgeFlag = rootPackageJson?.appConfig?.makeOrder?.ableHedgeFlag;
  const ableHedgeFlagResolved =
    ableHedgeFlag == undefined ? true : ableHedgeFlag;
  return ableHedgeFlagResolved;
};

export const getT0Config = (): {
  instrumentTypes: InstrumentTypeEnum[];
  exchangeIds: string[];
} => {
  const rootPackageJson = readRootPackageJsonSync();
  const T0Config =
    rootPackageJson?.appConfig?.T0T1?.T0 || ({} as T0T1Config['T0']);
  const instrumentTypes = (
    Array.isArray(T0Config.instrumentTypes) ? T0Config.instrumentTypes : []
  ).map((item) => InstrumentTypeEnum[item]);
  const exchangeIds = Array.isArray(T0Config.exchangeIds)
    ? T0Config.exchangeIds
    : [];
  return {
    instrumentTypes,
    exchangeIds,
  };
};

export const isT0 = (
  instrumentType: InstrumentTypeEnum,
  exchangeId?: string,
): boolean => {
  const { instrumentTypes, exchangeIds } = getT0Config();
  return (
    (instrumentType
      ? [...T0InstrumentTypes, ...instrumentTypes].includes(instrumentType)
      : false) ||
    (exchangeId
      ? [...T0ExchangeIds, ...exchangeIds].includes(exchangeId)
      : false)
  );
};

export const isKfColor = (color: string) => color.startsWith('kf-color');

export const isHexOrRgbColor = (color: string) =>
  color.startsWith('#') || color.startsWith('rgb') || color.startsWith('rgba');

export const dealKfNumber = (
  preNumber: bigint | number | undefined | unknown,
): string | number | bigint | unknown => {
  if (preNumber === undefined) return '--';
  if (preNumber === null) return '--';

  if (Number.isNaN(Number(preNumber))) {
    return '--';
  }
  return preNumber;
};

export const dealKfPrice = (
  preNumber: bigint | number | undefined | null | unknown,
): string => {
  const afterNumber = dealKfNumber(preNumber);

  if (afterNumber === '--') {
    return afterNumber;
  }

  return Number(afterNumber).toFixed(4);
};

export const dealAssetPrice = (
  preNumber: bigint | number | undefined | unknown,
): string => {
  const afterNumber = dealKfNumber(preNumber);

  if (afterNumber === '--') {
    return afterNumber;
  }

  return Number(afterNumber).toFixed(2);
};

export const sum = (list: number[]): number => {
  if (!list.length) return 0;
  return list.reduce((accumlator, a) => accumlator + +a);
};

export const dealVolumeByInstrumentType = (
  volume: number,
  instrumentType: InstrumentTypeEnum,
) => {
  const minOrderVolume = InstrumentMinOrderVolume[instrumentType] || 1;
  const orderVolume = Math.max(volume, minOrderVolume);
  return ~~(orderVolume / minOrderVolume) * minOrderVolume;
};

export const dealSide = (
  side: SideEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return Side[+side as SideEnum];
};

export const dealOffset = (
  offset: OffsetEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return Offset[+offset as OffsetEnum];
};

export const dealDirection = (
  direction: DirectionEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return Direction[+direction as DirectionEnum];
};

export const resolveDirectionBySideAndOffset = (
  side: SideEnum,
  offset: OffsetEnum,
): DirectionEnum => {
  if (side === SideEnum.Buy) {
    return offset === OffsetEnum.Open
      ? DirectionEnum.Long
      : DirectionEnum.Short;
  } else if (side === SideEnum.Sell) {
    return offset === OffsetEnum.Open
      ? DirectionEnum.Short
      : DirectionEnum.Long;
  }

  return DirectionEnum.Long;
};

export const dealOrderStatus = (
  status: OrderStatusEnum | number,
  errorMsg?: string,
): KungfuApi.KfTradeValueCommonData => {
  return {
    ...OrderStatus[+status as OrderStatusEnum],
    ...(+status === OrderStatusEnum.Error && errorMsg
      ? {
          name: errorMsg,
        }
      : {}),
  };
};

export const dealPriceType = (
  priceType: PriceTypeEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return PriceType[+priceType as PriceTypeEnum];
};

export const dealPriceLevel = (
  priceLevel: PriceLevelEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return PriceLevel[+priceLevel as PriceLevelEnum];
};

export const dealTimeCondition = (
  timeCondition: TimeConditionEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return TimeCondition[+timeCondition as TimeConditionEnum];
};

export const dealVolumeCondition = (
  volumeCondition: VolumeConditionEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return VolumeCondition[+volumeCondition as VolumeConditionEnum];
};

export const dealCommissionMode = (
  commissionMode: CommissionModeEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return CommissionMode[+commissionMode as CommissionModeEnum];
};

export const dealInstrumentType = (
  instrumentType: InstrumentTypeEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return InstrumentType[+instrumentType as InstrumentTypeEnum];
};

export const dealHedgeFlag = (
  hedgeFlag: HedgeFlagEnum | number,
): KungfuApi.KfTradeValueCommonData => {
  return HedgeFlag[+hedgeFlag as HedgeFlagEnum];
};

export const dealIsSwap = (isSwap: boolean) => {
  return { name: isSwap ? t('yes') : t('no') };
};

export const dealUnderweightType = (underweightType: UnderweightEnum) => {
  return UnderweightType[+underweightType as UnderweightEnum];
};

export const getKfCategoryData = (
  category: KfCategoryTypes,
): KungfuApi.KfTradeValueCommonData => {
  if (KfCategory[KfCategoryEnum[category]]) {
    return KfCategory[KfCategoryEnum[category]];
  }

  throw new Error(`Category ${category} is illegal`);
};

export const dealCategory = (
  category: KfCategoryTypes,
  extraCategory: Record<string, KungfuApi.KfTradeValueCommonData>,
): KungfuApi.KfTradeValueCommonData => {
  return KfCategory[KfCategoryEnum[category]] || extraCategory[category];
};

export const dealOrderStat = (
  orderStats: KungfuApi.DataTable<KungfuApi.OrderStat>,
  orderUKey: string,
): {
  latencySystem: string;
  latencyNetwork: string;
  latencyTrade: string;
  trade_time: bigint;
  avg_price: number;
} | null => {
  const orderStat = orderStats[orderUKey];
  if (!orderStat) {
    return null;
  }

  const { insert_time, ack_time, md_time, trade_time } = orderStat;
  const latencyTrade =
    trade_time && ack_time
      ? Number(Number(trade_time - ack_time) / 1000).toFixed(0)
      : '--';
  const latencyNetwork =
    ack_time && insert_time
      ? Number(Number(ack_time - insert_time) / 1000).toFixed(0)
      : '--';
  const latencySystem =
    insert_time && md_time
      ? Number(Number(insert_time - md_time) / 1000).toFixed(0)
      : '--';

  return {
    latencySystem,
    latencyNetwork,
    latencyTrade,
    trade_time: orderStat.trade_time,
    avg_price: orderStat.avg_price,
  };
};

export const dealLocationUID = (
  watcher: KungfuApi.Watcher | null,
  uid: number,
): string => {
  if (!watcher) {
    return '--';
  }

  const kfLocation = watcher?.getLocation(uid);
  if (!kfLocation) return '';
  return getIdByKfLocation(kfLocation);
};

export const resolveAccountId = (
  watcher: KungfuApi.Watcher | null,
  source: number,
  dest: number,
): KungfuApi.KfTradeValueCommonData => {
  if (!watcher) return { color: 'default', name: '--' };

  const accountId = dealLocationUID(watcher, source);
  const destLocation: KungfuApi.KfLocation = watcher.getLocation(dest);

  if (destLocation && destLocation.group === 'node') {
    return {
      color: 'orange',
      name: `${accountId} ${t('手动')}`,
    };
  }

  return {
    color: 'text',
    name: accountId,
  };
};

export const resolveClientId = (
  watcher: KungfuApi.Watcher | null,
  dest: number,
): KungfuApi.KfTradeValueCommonData => {
  if (!watcher) return { color: 'default', name: '--' };

  if (dest === 0) {
    return { color: 'default', name: t('系统外') };
  }

  const destLocation: KungfuApi.KfLocation = watcher.getLocation(dest);
  if (!destLocation) return { color: 'default', name: '--' };
  const destUname = getIdByKfLocation(destLocation);

  if (destLocation.group === 'node') {
    return { color: 'orange', name: t('手动') };
  } else {
    return { color: 'text', name: destUname };
  }
};

export const getOrderTradeFilterKey = (category: KfCategoryTypes): string => {
  if (category === 'td') {
    return 'source';
  } else if (category === 'strategy') {
    return 'dest';
  }

  return '';
};

export const getTradingDataSortKey = (
  typename: KungfuApi.TradingDataTypeName,
): string => {
  switch (typename) {
    case 'Order':
      return 'insert_time';
    case 'Trade':
      return 'trade_time';
    case 'OrderInput':
      return 'insert_time';
    case 'Position':
      return 'instrument_id';
    default:
      return '';
  }
};

export const getLedgerCategory = (category: KfCategoryTypes): 0 | 1 => {
  if (category !== 'td' && category !== 'strategy') {
    return LedgerCategoryEnum.td;
  }

  return LedgerCategoryEnum[category as LedgerCategoryTypes];
};

export const dealAppStates = (
  watcher: KungfuApi.Watcher | null,
  appStates: Record<string, BrokerStateStatusEnum>,
): Record<string, BrokerStateStatusTypes> => {
  if (!watcher) {
    return {} as Record<string, BrokerStateStatusTypes>;
  }

  return Object.keys(appStates || {}).reduce((appStatesResolved, key) => {
    const kfLocation = watcher.getLocation(key);
    const processId = getProcessIdByKfLocation(kfLocation);
    const appStateValue = appStates[key];
    appStatesResolved[processId] = BrokerStateStatusEnum[
      appStateValue
    ] as BrokerStateStatusTypes;
    return appStatesResolved;
  }, {} as Record<string, BrokerStateStatusTypes>);
};

export const dealStrategyStates = (
  watcher: KungfuApi.Watcher | null,
  strategyStates: Record<string, KungfuApi.StrategyStateDataOrigin>,
): Record<string, KungfuApi.StrategyStateData> => {
  if (!watcher) {
    return {} as Record<string, KungfuApi.StrategyStateDataOrigin>;
  }

  return Object.keys(strategyStates || {}).reduce(
    (strategyStatesResolved, key) => {
      const kfLocation = watcher.getLocation(key);
      const processId = getProcessIdByKfLocation(kfLocation);
      const strategyStateValue = deepClone(strategyStates[key]);
      strategyStateValue.state = StrategyStateStatusEnum[
        strategyStateValue.state
      ] as StrategyStateStatusTypes;
      strategyStatesResolved[processId] =
        strategyStateValue as KungfuApi.StrategyStateData;
      return strategyStatesResolved;
    },
    {} as Record<string, KungfuApi.StrategyStateData>,
  );
};

export const dealAssetsByHolderUID = <
  T extends KungfuApi.Asset | KungfuApi.AssetMargin,
>(
  watcher: KungfuApi.Watcher | null,
  assets: KungfuApi.DataTable<T>,
): Record<string, T> => {
  if (!watcher) {
    return {} as Record<string, T>;
  }

  return Object.values(assets).reduce((assetsResolved, asset) => {
    const { holder_uid } = asset;
    const kfLocation = watcher.getLocation(holder_uid);

    if (kfLocation) {
      const processId = getProcessIdByKfLocation(kfLocation);
      assetsResolved[processId] = asset;
    }

    return assetsResolved;
  }, {} as Record<string, T>);
};

export const dealOrderTradingData = <T>(
  watcher: KungfuApi.Watcher,
  tradingData: KungfuApi.DataTable<T>,
  tradingDataTypeName: KungfuApi.TradingDataTypeName,
  kfLocation: KungfuApi.KfLocation,
): T[] => {
  const currentUID = watcher.getLocationUID(kfLocation);
  const orderTradeFilterKey = getOrderTradeFilterKey(kfLocation.category);
  const sortKey = getTradingDataSortKey(tradingDataTypeName);

  const afterFilterDatas = tradingData.filter(orderTradeFilterKey, currentUID);

  if (sortKey) {
    return afterFilterDatas.sort(sortKey);
  } else {
    return afterFilterDatas.list();
  }
};

export const dealLedgerTradingData = <T>(
  watcher: KungfuApi.Watcher,
  tradingData: KungfuApi.DataTable<T>,
  tradingDataTypeName: KungfuApi.TradingDataTypeName,
  kfLocation: KungfuApi.KfLocation,
): T[] => {
  const sortKey = getTradingDataSortKey(tradingDataTypeName);

  const { category } = kfLocation;
  const ledgerCategory = getLedgerCategory(category);
  const locationUID = watcher.getLocationUID(kfLocation);
  let dataTableResolved = tradingData;

  if (tradingDataTypeName === 'Position') {
    dataTableResolved = dataTableResolved.nofilter('volume', BigInt(0));
  }

  dataTableResolved = dataTableResolved
    .filter('ledger_category', ledgerCategory)
    .filter('holder_uid', locationUID);

  if (sortKey) {
    return dataTableResolved.sort(sortKey);
  }

  return dataTableResolved.list();
};

export const dealDefaultTradingData = <T>(
  ...args: [
    KungfuApi.Watcher,
    KungfuApi.DataTable<T>,
    KungfuApi.TradingDataTypeName,
    KungfuApi.KfLocation,
  ]
): T[] => {
  return args[1].list();
};

export const dealTradingDataMethodsMap: Record<
  KungfuApi.TradingDataTypeName,
  <T>(
    watcher: KungfuApi.Watcher,
    tradingData: KungfuApi.DataTable<T>,
    tradingDataTypeName: KungfuApi.TradingDataTypeName,
    kfLocation: KungfuApi.KfLocation,
  ) => T[]
> = {
  Asset: dealLedgerTradingData,
  AssetMargin: dealLedgerTradingData,
  Instrument: dealDefaultTradingData,
  Order: dealOrderTradingData,
  OrderInput: dealOrderTradingData,
  OrderStat: dealDefaultTradingData,
  Position: dealLedgerTradingData,
  Quote: dealDefaultTradingData,
  Trade: dealOrderTradingData,
  Basket: dealDefaultTradingData,
  BasketInstrument: dealDefaultTradingData,
  BasketOrder: dealDefaultTradingData,
};

export const dealTradingData = <T>(
  watcher: KungfuApi.Watcher | null,
  tradingData: KungfuApi.DataTable<T>,
  tradingDataTypeName: KungfuApi.TradingDataTypeName,
  kfLocation: KungfuApi.KfLocation,
): T[] => {
  if (!watcher) {
    throw new Error('Watcher is NULL');
  }

  return dealTradingDataMethodsMap[tradingDataTypeName]<T>(
    watcher,
    tradingData,
    tradingDataTypeName,
    kfLocation,
  );
};

export const getPrimaryKeyFromKfConfigItem = (
  settings: KungfuApi.KfConfigItem[],
): KungfuApi.KfConfigItem[] => {
  return settings.filter((item) => {
    return !!item.primary;
  });
};

export const getPrimaryKeys = (
  settings: KungfuApi.KfConfigItem[],
): string[] => {
  return settings.filter((item) => item.primary).map((item) => item.key);
};

export const getCombineValueByPrimaryKeys = (
  primaryKeys: string[],
  formState: Record<string, KungfuApi.KfConfigValue>,
  extraValue = '',
) => {
  return [extraValue || '', ...primaryKeys.map((key) => formState[key])]
    .filter((item) => item !== '')
    .join('_');
};

export const transformSearchInstrumentResultToInstrument = (
  instrumentStr: string,
): KungfuApi.InstrumentResolved | null => {
  const pair = instrumentStr.split('_');
  if (pair.length !== 5) return null;
  const [exchangeId, instrumentId, instrumentType, ukey, instrumentName] = pair;
  return {
    exchangeId,
    instrumentId,
    instrumentType: +instrumentType as InstrumentTypeEnum,
    instrumentName,
    id: `${instrumentId}_${instrumentName}_${exchangeId}`.toLowerCase(),
    ukey,
  };
};

export const booleanProcessEnv = (
  val: string | boolean | undefined,
): boolean => {
  if (val === undefined) {
    return false;
  }

  if (val === 'null') {
    return false;
  }

  if (val === 'true') {
    return true;
  } else if (val === 'false') {
    return false;
  } else {
    return !!val;
  }
};

export const numberEnumRadioType: Record<
  string,
  Record<number, KungfuApi.KfTradeValueCommonData>
> = {
  offset: getOffsetConfig(),
  hedgeFlag: HedgeFlag,
  direction: Direction,
  volumeCondition: VolumeCondition,
  timeCondition: TimeCondition,
  commissionMode: CommissionMode,
};

export const numberEnumSelectType: Record<
  string,
  Record<number, KungfuApi.KfTradeValueCommonData>
> = {
  side: Side,
  priceType: PriceType,
  priceLevel: PriceLevel,
  instrumentType: InstrumentType,
  underweightType: UnderweightType,
};

export const stringEnumSelectType: Record<
  string,
  Record<string, KungfuApi.KfTradeValueCommonData>
> = {
  exchange: ExchangeIds,
  futureArbitrageCode: FutureArbitrageCodes,
};

export const KfConfigValueNumberType = [
  'int',
  'float',
  'percent',
  ...Object.keys(numberEnumSelectType || {}),
  ...Object.keys(numberEnumRadioType || {}),
];

export const KfConfigValueBooleanType = ['bool', 'checkbox'];

export const KfConfigValueArrayType = [
  'tds',
  'files',
  'instruments',
  'instrumentsCsv',
  'table',
  'csvTable',
];

export const initFormTimePicker = (initValue?: string) => {
  if (typeof initValue !== 'string') return null;

  let parsedValue: dayjs.Dayjs | null = null;

  if (initValue === 'now') {
    parsedValue = dayjs();
  } else if (/\d{2}:\d{2}:\d{2}/.test(initValue)) {
    parsedValue = dayjs(initValue, 'HH:mm:ss');
  } else if (/\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}/.test(initValue)) {
    parsedValue = dayjs(initValue, 'YYYY-MM-DD HH:mm:ss');
  }

  if (parsedValue) return parsedValue.format('YYYY-MM-DD HH:mm:ss');

  return null;
};

export const initFormStateByConfig = (
  configSettings: KungfuApi.KfConfigItem[],
  initValue?: Record<string, KungfuApi.KfConfigValue>,
): Record<string, KungfuApi.KfConfigValue> => {
  if (!configSettings) return {};
  const formState: Record<string, KungfuApi.KfConfigValue> = {};
  configSettings.forEach((item) => {
    const type = item.type;
    const isBoolean = KfConfigValueBooleanType.includes(type);
    const isNumber = KfConfigValueNumberType.includes(type);
    const isArray = KfConfigValueArrayType.includes(type);

    let defaultValue;

    const getDefaultValueByType = () => {
      return isBoolean
        ? false
        : isNumber
        ? 0
        : type === 'timePicker'
        ? null
        : isArray
        ? []
        : '';
    };

    if (typeof item?.default === 'object') {
      defaultValue = JSON.parse(JSON.stringify(item?.default));
    } else {
      defaultValue = item?.default;
    }

    if (
      (initValue || {})[item.key] !== undefined &&
      (initValue || {})[item.key] !== item?.default
    ) {
      defaultValue = (initValue || {})[item.key];
    }

    if (defaultValue === undefined) {
      defaultValue = getDefaultValueByType();
    }

    if (KfConfigValueBooleanType.includes(type)) {
      defaultValue =
        defaultValue === 'true'
          ? true
          : defaultValue === 'false'
          ? false
          : !!defaultValue;
    } else if (KfConfigValueNumberType.includes(type)) {
      defaultValue = +defaultValue;
    } else if (KfConfigValueArrayType.includes(type)) {
      if (typeof defaultValue === 'string') {
        try {
          defaultValue = JSON.parse(defaultValue);
        } catch (err) {
          defaultValue = [];
        }
      }
    } else if (item.type === 'timePicker') {
      defaultValue = initFormTimePicker(item?.default);
    }

    formState[item.key] = defaultValue;
  });

  return formState;
};

export const resolveInstrumentValue = (
  type: 'instrument' | 'instruments' | 'instrumentsCsv',
  value: string | string[],
): string[] => {
  if (type === 'instruments' || type === 'instrumentsCsv') {
    return (value || ['']) as string[];
  }
  if (type === 'instrument') {
    return [(value || '') as string];
  } else {
    return [];
  }
};

//深度克隆obj
export const deepClone = <T>(obj: T): T => {
  if (!obj) return obj;
  return JSON.parse(JSON.stringify(obj));
};

export function isCriticalLog(line: string): boolean {
  if (line.indexOf('critical') !== -1) {
    return true;
  }

  if (line.indexOf('File') !== -1) {
    if (line.indexOf('line') !== -1) {
      return true;
    }
  }

  if (line.indexOf('Traceback') != -1) {
    return true;
  }

  if (line.indexOf(' Error ') != -1) {
    return true;
  }

  if (line.indexOf('Try') != -1) {
    if (line.indexOf('for help') != -1) {
      return true;
    }
  }

  if (line.indexOf('Usage') != -1) {
    return true;
  }

  if (line.indexOf('Failed to execute') != -1) {
    return true;
  }

  if (line.indexOf('KeyboardInterrupt') != -1) {
    return true;
  }

  return false;
}

export const removeNoDefaultStrategyFolders = async (): Promise<void> => {
  const strategyDir = path.join(KF_RUNTIME_DIR, 'strategy');
  const filedirList: string[] = (await listDir(strategyDir)) || [];
  filedirList.map((fileOrFolder) => {
    const fullPath = path.join(strategyDir, fileOrFolder);
    if (fileOrFolder === 'default') {
      if (fse.statSync(fullPath).isDirectory()) {
        return Promise.resolve();
      }
    }
    return fse.remove(fullPath);
  });
};

// 处理下单时输入数据
export const dealOrderInputItem = (
  inputData: KungfuApi.MakeOrderInput,
): Record<string, KungfuApi.KfTradeValueCommonData> => {
  const orderInputResolved: Record<string, KungfuApi.KfTradeValueCommonData> =
    {};
  const isInstrumnetShotable = isShotable(inputData.instrument_type);
  for (const key in inputData) {
    if (key === 'instrument_type') {
      orderInputResolved[key] = dealInstrumentType(inputData.instrument_type);
    } else if (key === 'price_type') {
      orderInputResolved[key] = dealPriceType(inputData.price_type);
    } else if (key === 'side') {
      orderInputResolved[key] = dealSide(inputData.side);
    } else if (key === 'offset') {
      isInstrumnetShotable &&
        (orderInputResolved[key] = dealOffset(inputData.offset));
    } else if (key === 'hedge_flag') {
      isInstrumnetShotable &&
        (orderInputResolved[key] = dealHedgeFlag(inputData.hedge_flag));
    } else if (key === 'is_swap') {
      isInstrumnetShotable &&
        (orderInputResolved[key] = dealIsSwap(inputData.is_swap));
    } else {
      orderInputResolved[key] = {
        name: inputData[key],
        color: 'default',
      };
    }
  }
  return orderInputResolved;
};

export const kfConfigItemsToProcessArgs = (
  settings: KungfuApi.KfConfigItem[],
  formState: Record<string, KungfuApi.KfConfigValue>,
): string => {
  return JSON.stringify(
    settings
      .filter((item) => {
        return formState[item.key] !== undefined;
      })
      .reduce((pre, item) => {
        pre[item.key] = formState[item.key];
        return pre;
      }, {} as Record<string, KungfuApi.KfConfigValue>),
  );
};

export const dealByConfigItemType = (
  type: string,
  value: KungfuApi.KfConfigValue,
  options?: KungfuApi.KfSelectOption[],
): string => {
  switch (type) {
    case 'side':
      return dealSide(+value as SideEnum).name;
    case 'offset':
      return dealOffset(+value as OffsetEnum).name;
    case 'direction':
      return dealDirection(+value as DirectionEnum).name;
    case 'priceType':
      return dealPriceType(+value as PriceTypeEnum).name;
    case 'priceLevel':
      return dealPriceLevel(+value as PriceLevelEnum).name;
    case 'hedgeFlag':
      return dealHedgeFlag(+value as HedgeFlagEnum).name;
    case 'volumeCondition':
      return dealVolumeCondition(+value as VolumeConditionEnum).name;
    case 'timeCondition':
      return dealTimeCondition(+value as TimeConditionEnum).name;
    case 'commissionMode':
      return dealCommissionMode(+value as CommissionModeEnum).name;
    case 'instrumentType':
      return dealInstrumentType(+value as InstrumentTypeEnum).name;
    case 'instrument':
      const instrumentData = transformSearchInstrumentResultToInstrument(value);
      return instrumentData
        ? `${instrumentData.exchangeId} ${instrumentData.instrumentId} ${
            instrumentData.instrumentName || ''
          }`
        : value;
    case 'instruments':
      const instrumentDataList = (value as string[])
        .map((item) => transformSearchInstrumentResultToInstrument(item))
        .filter((item) => !!item) as KungfuApi.InstrumentResolved[];
      return instrumentDataList
        .map(
          (item) =>
            `${item.exchangeId} ${item.instrumentId} ${
              item.instrumentName || ''
            }`,
        )
        .join(' ');
    case 'select':
    case 'radio':
      if (!options?.length) return value;
      const { isLanguageKeyAvailable } = useLanguage();
      const label = options.filter((option) => option.value === value)[0]
        .label as string;
      return isLanguageKeyAvailable(label) ? t(label) : label;
    default:
      return value;
  }
};

export const kfConfigItemsToArgsByPrimaryForShow = (
  settings: KungfuApi.KfConfigItem[],
  formState: Record<string, KungfuApi.KfConfigValue>,
): string => {
  const { isLanguageKeyAvailable } = useLanguage();
  return settings
    .filter((item) => item.primary)
    .map((item) => ({
      label: isLanguageKeyAvailable(item.name) ? t(item.name) : item.name,
      value: dealByConfigItemType(item.type, formState[item.key], item.options),
    }))
    .map((item) => `${item.label} ${item.value}`)
    .join('; ');
};

export const fromProcessArgsToKfConfigItems = (
  args: string[],
): Record<string, KungfuApi.KfConfigValue> => {
  const taskArgs = minimist(args)['a'] || '{}';
  const data = JSON.parse(taskArgs);
  return data;
};

export const getTaskListFromProcessStatusData = (
  taskPrefixs: string[],
  psDetail: Pm2ProcessStatusDetailData,
): Pm2ProcessStatusDetail[] => {
  return Object.keys(psDetail)
    .filter((processId) => {
      return (
        taskPrefixs.findIndex((cg) => {
          return processId.indexOf(cg) === 0;
        }) !== -1
      );
    })
    .map((processId) => psDetail[processId])
    .sort((a, b) => {
      const aCreateTime = +(a.name?.toKfName() || 0);
      const bCreateTime = +(b.name?.toKfName() || 0);
      return aCreateTime - bCreateTime;
    });
};

export function dealTradingTaskName(
  name: string,
  extConfigs: KungfuApi.KfExtConfigs,
): string {
  const { isLanguageKeyAvailable } = useLanguage();
  const group = name.toKfGroup();
  const strategyExts = extConfigs['strategy'] || {};
  const groupResolved = strategyExts[group] ? strategyExts[group].name : group;
  const groupTranslated = isLanguageKeyAvailable(groupResolved)
    ? t(groupResolved)
    : groupResolved;
  const timestamp = name.toKfName();
  return `${groupTranslated} ${dayjs(+timestamp).format('HH:mm:ss')}`;
}

export const isBrokerStateReady = (state: BrokerStateStatusTypes) => {
  return state === 'Ready' || state === 'Idle';
};

export function deleteNNFiles(rootPathName = KF_HOME) {
  return new Promise((resolve, reject) => {
    glob(
      '**/*.nn',
      {
        cwd: rootPathName,
      },
      (err: Error | null, files: string[]) => {
        if (err) {
          reject(err);
        }

        files.forEach((file: string) => {
          unlinkSync(path.join(rootPathName, file));
        });

        resolve(true);
      },
    );
  });
}

export const dealCmdPath = (pathname: string) => {
  if (os.platform() === 'win32') {
    return pathname
      .replace(/\\/g, '/')
      .split('/')
      .map((str) => {
        if (str.includes(' ')) {
          return `"${str}"`;
        }

        return str;
      })
      .join('/');
  }
  return pathname;
};

export const isUpdateVersionLogicEnable = () => {
  const packageJson = readRootPackageJsonSync();
  console.log(
    'packageJson?.kungfuCraft?.autoUpdate?.update',
    packageJson?.kungfuCraft?.autoUpdate?.update,
  );
  if (!packageJson?.kungfuCraft?.autoUpdate?.update) return false;
  const globalSetting = getKfGlobalSettingsValue();
  console.log(
    'globalSetting?.update?.isCheckVersion',
    globalSetting?.update?.isCheckVersion,
  );
  if (!globalSetting?.update?.isCheckVersion) return false;
  return true;
};
