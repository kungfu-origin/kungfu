import path from 'path';
import dayjs from 'dayjs';
import fse, { Stats } from 'fs-extra';
import log4js from 'log4js';
import {
  buildProcessLogPath,
  EXTENSION_DIRS,
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
  MakeOrderByWatcherEnum,
  BrokerStateStatusEnum,
  StrategyExtTypes,
} from '../typings/enums';
import {
  deleteProcess,
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
import { listDir, removeTargetFilesInFolder } from './fileUtils';
import minimist from 'minimist';

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

export {};

export const getGlobal = () => {
  if (typeof self !== 'undefined') {
    return self;
  }
  if (typeof window !== 'undefined') {
    return window;
  }
  if (typeof global !== 'undefined') {
    return global;
  }
  throw new Error('unable to locate global object');
};

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
  const t = this;
  const len = t.length;
  let i = 0;

  while (i < len) {
    cb.call(t, t[i], i);
    i++;
  }
};

Array.prototype.kfReverseForEach = function (cb) {
  if (!cb) return;
  const t = this;
  let i = t.length;
  while (i--) {
    cb.call(t, t[i], i);
  }
};

Array.prototype.kfForEachAsync = function (cb) {
  if (!cb) return;
  const t = this;
  const len = t.length;
  return new Promise((resolve) => {
    setImmediateIter(t, 0, len, cb, () => {
      resolve(true);
    });
  });
};

function setImmediateIter<T>(
  list: Array<T>,
  i: number,
  len: number,
  cb: Function,
  fcb: Function,
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
  info: (...args: Array<string | number>) => {
    if (process.env.NODE_ENV === 'development') {
      if (process.env.APP_TYPE !== 'cli') {
        console.log('<KF_INFO>', args.join(' '));
      }
    }
    logger.info('<KF_INFO>', args.join(' '));
  },

  warn: (...args: Array<string | number>) => {
    if (process.env.NODE_ENV === 'development') {
      if (process.env.APP_TYPE !== 'cli') {
        console.warn('<KF_INFO>', args.join(' '));
      }
    }
    logger.warn('<KF_INFO>', args.join(' '));
  },

  error: (...args: Array<string | number>) => {
    if (process.env.NODE_ENV === 'development') {
      if (process.env.APP_TYPE !== 'cli') {
        console.error('<KF_INFO>', args.join(' '));
      }
    }
    logger.error('<KF_INFO>', args.join(' '));
  },
};

export const dealSpaceInPath = (pathname: string) => {
  const normalizePath = path.normalize(pathname);
  return normalizePath.replace(/ /g, ' ');
};

export const setTimerPromiseTask = (fn: Function, interval = 500) => {
  var taskTimer: number | undefined = undefined;
  var clear = false;
  function timerPromiseTask(fn: Function, interval = 500) {
    if (taskTimer) global.clearTimeout(taskTimer as unknown as NodeJS.Timeout);
    fn().finally(() => {
      if (clear) {
        if (taskTimer)
          global.clearTimeout(taskTimer as unknown as NodeJS.Timeout);
        return;
      }
      taskTimer = +global.setTimeout(() => {
        timerPromiseTask(fn, interval);
      }, interval);
    });
  }
  timerPromiseTask(fn, interval);
  return {
    clearLoop: function () {
      clear = true;
      if (taskTimer != null)
        global.clearTimeout(taskTimer as unknown as NodeJS.Timeout);
    },
  };
};

export const loopToRunProcess = async <T>(
  promiseFunc: Array<() => Promise<T>>,
  interval = 100,
) => {
  let i = 0,
    len = promiseFunc.length;
  let resList: (T | Error)[] = [];
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
    let timer = setTimeout(() => {
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

  let i = 0,
    len = statsList.length;
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
      const components = uiConfig?.components;
      const daemon = uiConfig?.daemon || ({} as Record<string, string>);
      const script = uiConfig?.script || '';

      configByExtraKey[extKey] = {
        name: extName,
        extPath,
        position,
        components: components || {
          index: 'index.js',
        },
        daemon,
        script,
      };
      return configByExtraKey;
    }, {} as KungfuApi.KfUIExtConfigs);
  2;
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
  let configCopy = JSON.parse(config);
  Object.keys(configCopy || {}).forEach((key: string) => {
    if (key.includes('password')) {
      configCopy[key] = '******';
    }
  });
  return JSON.stringify(configCopy);
};

export const getTradingDate = (today = true): string => {
  if (today) {
    return dayjs().format('YYYY-MM-DD');
  }

  const currentTimestamp = dayjs().valueOf();
  const tradingDayTimestamp = +dayjs()
    .set('hour', 15)
    .set('minute', 30)
    .valueOf();

  if (currentTimestamp > tradingDayTimestamp) {
    return dayjs().add(1, 'day').format('YYYY-MM-DD');
  } else {
    return dayjs().format('YYYY-MM-DD');
  }
};

export const removeJournal = (targetFolder: string): Promise<void> => {
  return removeTargetFilesInFolder(targetFolder, ['.journal']);
};

export const removeDB = (targetFolder: string): Promise<void> => {
  return removeTargetFilesInFolder(targetFolder, ['.db'], ['config.db']);
};

export const getProcessIdByKfLocation = (
  kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
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

export const getTaskKfLocationByProcessId = (
  processId: string,
): KungfuApi.KfLocation | null => {
  if (processId.indexOf('strategy_') === 0) {
    const [category, group] = processId.split('_');
    return {
      category: category as KfCategoryTypes,
      group,
      name: processId.split('_').slice(2).join('_'),
      mode: 'live',
    };
  }

  return null;
};

export const getIdByKfLocation = (
  kfLocation:
    | KungfuApi.KfLocation
    | KungfuApi.KfConfig
    | KungfuApi.KfExtraLocation,
): string => {
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
  kfConfig: KungfuApi.KfLocation | KungfuApi.KfConfig,
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

export const getPropertyFromProcessStatusDetailDataByKfLocation = (
  processStatusDetailData: Pm2ProcessStatusDetailData,
  kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
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

export const debounce = (fn: Function, delay = 300, immediate = false) => {
  let timeout: number;
  return (...args: any) => {
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

export const switchKfLocation = (
  watcher: KungfuApi.Watcher | null,
  kfLocation:
    | KungfuApi.KfLocation
    | KungfuApi.KfConfig
    | KungfuApi.KfExtraLocation,
  targetStatus: boolean,
): Promise<void | Proc> => {
  const processId = getProcessIdByKfLocation(kfLocation);

  if (!targetStatus) {
    if (
      kfLocation.category === 'td' ||
      kfLocation.category === 'md' ||
      kfLocation.category === 'strategy'
    ) {
      if (watcher && !watcher.isReadyToInteract(kfLocation)) {
        return Promise.reject(
          new Error(`${processId} 还未准备就绪, 请稍后重试`),
        );
      }
    }

    return deleteProcess(processId);
  }

  switch (kfLocation.category) {
    case 'system':
      if (kfLocation.name === 'master') {
        return startMaster(true);
      } else if (kfLocation.name === 'ledger') {
        return startLedger(true);
      } else if (kfLocation.name === 'cached') {
        return startCacheD(true);
      }

    case 'td':
      return startTd(getIdByKfLocation(kfLocation));
    case 'md':
      return startMd(getIdByKfLocation(kfLocation));
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

export const dealSide = (side: SideEnum): KungfuApi.KfTradeValueCommonData => {
  return Side[side];
};

export const dealOffset = (
  offset: OffsetEnum,
): KungfuApi.KfTradeValueCommonData => {
  return Offset[offset];
};

export const dealDirection = (
  direction: DirectionEnum,
): KungfuApi.KfTradeValueCommonData => {
  return Direction[direction];
};

export const dealOrderStatus = (
  status: OrderStatusEnum,
  errorMsg?: string,
): KungfuApi.KfTradeValueCommonData => {
  return {
    ...OrderStatus[status],
    ...(+status === OrderStatusEnum.Error && errorMsg
      ? {
          name: errorMsg,
        }
      : {}),
  };
};

export const dealPriceType = (
  priceType: PriceTypeEnum,
): KungfuApi.KfTradeValueCommonData => {
  return PriceType[priceType];
};

export const dealTimeCondition = (
  timeCondition: TimeConditionEnum,
): KungfuApi.KfTradeValueCommonData => {
  return TimeCondition[timeCondition];
};

export const dealVolumeCondition = (
  volumeCondition: VolumeConditionEnum,
): KungfuApi.KfTradeValueCommonData => {
  return VolumeCondition[volumeCondition];
};

export const dealInstrumentType = (
  instrumentType: InstrumentTypeEnum,
): KungfuApi.KfTradeValueCommonData => {
  return InstrumentType[instrumentType];
};

export const dealHedgeFlag = (
  hedgeFlag: HedgeFlagEnum,
): KungfuApi.KfTradeValueCommonData => {
  return HedgeFlag[hedgeFlag];
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
  parent_id: bigint,
): KungfuApi.KfTradeValueCommonData => {
  if (!watcher) return { color: 'default', name: '--' };

  const accountId = dealLocationUID(watcher, source);
  const destLocation: KungfuApi.KfLocation = watcher.getLocation(dest);

  if (parent_id === BigInt(MakeOrderByWatcherEnum.Manual)) {
    return {
      color: 'orange',
      name: `${accountId} 手动`,
    };
  }

  if (destLocation && destLocation.group === 'node') {
    if (parent_id !== BigInt(0)) {
      return {
        color: 'blue',
        name: `${accountId} 任务`,
      };
    }
  }

  return {
    color: 'text',
    name: accountId,
  };
};

export const resolveClientId = (
  watcher: KungfuApi.Watcher | null,
  dest: number,
  parent_id: bigint,
): KungfuApi.KfTradeValueCommonData => {
  if (!watcher) return { color: 'default', name: '--' };

  if (dest === 0) {
    return { color: 'default', name: '系统外' };
  }

  const destLocation: KungfuApi.KfLocation = watcher.getLocation(dest);
  if (!destLocation) return { color: 'default', name: '--' };

  if (destLocation.group === 'node') {
    if (parent_id !== BigInt(0)) {
      return { color: 'blue', name: '任务' };
    } else {
      return { color: 'orange', name: '手动' };
    }
  } else {
    if (parent_id !== BigInt(0)) {
      return { color: 'orange', name: `${destLocation.name} 手动` }; //是因为策略模块手动下单的时候刻意插入用于区分
    } else {
      return { color: 'text', name: destLocation.name };
    }
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
  if (typename === 'Order') {
    return 'update_time';
  } else if (typename === 'Trade') {
    return 'trade_time';
  } else if (typename === 'OrderInput') {
    return 'insert_time';
  } else if (typename === 'Position') {
    return 'instrument_id';
  }

  return '';
};

export const getLedgerCategory = (category: KfCategoryTypes): 0 | 1 => {
  if (category !== 'td' && category !== 'strategy') {
    return LedgerCategoryEnum.td;
  }

  return LedgerCategoryEnum[category as LedgerCategoryTypes];
};

export const filterLedgerResult = <T>(
  dataTable: KungfuApi.DataTable<T>,
  tradingDataTypeName: KungfuApi.TradingDataTypeName,
  kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
  sortKey?: string,
): T[] => {
  const { category, group, name } = kfLocation;
  const ledgerCategory = getLedgerCategory(category);
  let dataTableResolved = dataTable;

  if (ledgerCategory !== undefined) {
    dataTableResolved = dataTable.filter('ledger_category', ledgerCategory);
  }

  if (tradingDataTypeName === 'Position') {
    dataTableResolved = dataTableResolved.nofilter('volume', BigInt(0));
  }

  if (ledgerCategory === 0) {
    dataTableResolved = dataTableResolved
      .filter('source_id', group)
      .filter('account_id', name);
  } else if (ledgerCategory === 1) {
    dataTableResolved = dataTableResolved.filter('client_id', name);
  }

  if (sortKey) {
    return dataTableResolved.sort(sortKey);
  }

  return dataTableResolved.list();
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
    const appStateValue = appStates[key] as BrokerStateStatusEnum;
    appStatesResolved[processId] = BrokerStateStatusEnum[
      appStateValue
    ] as BrokerStateStatusTypes;
    return appStatesResolved;
  }, {} as Record<string, BrokerStateStatusTypes>);
};

export const dealAssetsByHolderUID = (
  watcher: KungfuApi.Watcher | null,
  assets: KungfuApi.DataTable<KungfuApi.Asset>,
): Record<string, KungfuApi.Asset> => {
  if (!watcher) {
    return {} as Record<string, KungfuApi.Asset>;
  }

  return Object.values(assets).reduce((assetsResolved, asset) => {
    const { holder_uid } = asset;
    const kfLocation = watcher.getLocation(holder_uid);
    const processId = getProcessIdByKfLocation(kfLocation);
    assetsResolved[processId] = asset;
    return assetsResolved;
  }, {} as Record<string, KungfuApi.Asset>);
};

export const dealTradingData = (
  watcher: KungfuApi.Watcher | null,
  tradingData: KungfuApi.TradingData | undefined,
  tradingDataTypeName: KungfuApi.TradingDataTypeName,
  kfLocation: KungfuApi.KfLocation | KungfuApi.KfConfig,
): KungfuApi.TradingDataNameToType[KungfuApi.TradingDataTypeName][] => {
  if (!watcher) {
    throw new Error('Watcher 错误');
  }

  if (!tradingData) {
    console.error('ledger is undefined');
    return [];
  }

  const currentUID = watcher.getLocationUID(kfLocation);
  const orderTradeFilterKey = getOrderTradeFilterKey(kfLocation.category);
  const sortKey = getTradingDataSortKey(tradingDataTypeName);

  if (
    tradingDataTypeName === 'Order' ||
    tradingDataTypeName === 'Trade' ||
    tradingDataTypeName === 'OrderInput'
  ) {
    const afterFilterDatas = tradingData[tradingDataTypeName].filter(
      orderTradeFilterKey,
      currentUID,
    );

    if (sortKey) {
      return afterFilterDatas.sort(sortKey);
    } else {
      return afterFilterDatas.list();
    }
  }

  return filterLedgerResult<
    KungfuApi.TradingDataNameToType[KungfuApi.TradingDataTypeName]
  >(tradingData[tradingDataTypeName], tradingDataTypeName, kfLocation, sortKey);
};

export const isTdStrategyCategory = (category: string): boolean => {
  if (category !== 'td') {
    if (category !== 'strategy') {
      return false;
    }
  }

  return true;
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

export const dealKfConfigValueByType = (
  type: KungfuApi.KfConfigItemSupportedTypes,
  value: KungfuApi.KfConfigValue,
): string => {
  switch (type) {
    case 'instrument':
      const instrumentResolved =
        transformSearchInstrumentResultToInstrument(value);
      if (!instrumentResolved) {
        return value;
      }
      const { exchangeId, instrumentId } = instrumentResolved;
      return `${exchangeId}_${instrumentId}`;
    case 'side':
      return dealSide(+value).name;
    case 'offset':
      return dealOffset(+value).name;
    case 'direction':
      return dealDirection(+value).name;
    case 'instrumentType':
      return dealInstrumentType(+value).name;
    case 'priceType':
      return dealPriceType(+value).name;
    case 'hedgeFlag':
      return dealHedgeFlag(+value).name;
    case 'volumeCondition':
      return dealVolumeCondition(+value).name;
    case 'timeCondition':
      return dealTimeCondition(+value).name;
    default:
      return value;
  }
};

export const booleanProcessEnv = (val: string): boolean => {
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
  offset: Offset,
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
  instrumentType: InstrumentType,
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

export const KfConfigValueBooleanType = ['bool'];

export const KfConfigValueArrayType = ['files', 'instruments', 'table'];

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
        ? dayjs().format('YYYY-MM-DD HH:mm:ss')
        : isArray
        ? []
        : '';
    };

    if (typeof item?.default === 'object') {
      defaultValue = JSON.parse(JSON.stringify(item?.default));
    } else {
      defaultValue = item?.default;
    }

    if (defaultValue === undefined) {
      defaultValue = getDefaultValueByType();
    }

    if (
      (initValue || {})[item.key] !== undefined &&
      (initValue || {})[item.key] !== getDefaultValueByType()
    ) {
      defaultValue = (initValue || {})[item.key];
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
    }

    formState[item.key] = defaultValue;
  });

  return formState;
};

export const resolveInstrumentValue = (
  type: 'instrument' | 'instruments',
  value: string | string[],
): string[] => {
  if (type === 'instruments') {
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
  for (let key in inputData) {
    if (key === 'instrument_type') {
      orderInputResolved[key] = dealInstrumentType(inputData.instrument_type);
    } else if (key === 'price_type') {
      orderInputResolved[key] = dealPriceType(inputData.price_type);
    } else if (key === 'side') {
      orderInputResolved[key] = dealSide(inputData.side);
    } else if (key === 'offset') {
      orderInputResolved[key] = dealOffset(inputData.offset);
    } else if (key === 'hedge_flag') {
      orderInputResolved[key] = dealHedgeFlag(inputData.hedge_flag);
    } else if (key === 'hedge_flag') {
      orderInputResolved[key] = dealHedgeFlag(inputData.hedge_flag);
    } else if (key === 'parent_id') {
      break;
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
) => {
  return settings
    .filter((item) => {
      return formState[item.key] !== undefined;
    })
    .map((item) => {
      return `${item.key}=${formState[item.key]}`;
    })
    .join(';');
};

export const fromProcessArgsToKfConfigItems = (
  args: string[],
): Record<string, KungfuApi.KfConfigValue> => {
  const taskArgs = minimist(args)['a'] || '';
  const data = getDataByProcessArgs(taskArgs);
  return data;
};

export const getDataByProcessArgs = (
  taskArgs: string,
): Record<string, string> => {
  return taskArgs.split(';').reduce((data, pair) => {
    const [key, value] = pair.split('=');
    data[key] = value;
    return data;
  }, {} as Record<string, string>);
};
