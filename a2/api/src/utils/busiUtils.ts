import path from 'path';
import dayjs from 'dayjs';
import fse, { Stats } from 'fs-extra';
import log4js from 'log4js';
import { buildProcessLogPath, EXTENSION_DIR } from '../config/pathConfig';
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
} from '../config/tradingConfig';
import {
    KfTradeValueCommonData,
    InstrumentTypeEnum,
    KfCategoryTypes,
    KfExtOriginConfig,
    KfExtConfigs,
    SourceData,
    ProcessStatusTypes,
    InstrumentTypes,
    KfConfig,
    KfLocation,
    KfConfigValue,
    KfCategoryEnum,
    BrokerStateStatusTypes,
    SideEnum,
    OffsetEnum,
    DirectionEnum,
    OrderStatusEnum,
    LedgerCategoryEnum,
    LedgerCategoryTypes,
    HedgeFlagEnum,
} from '../typings';
import {
    deleteProcess,
    Pm2ProcessStatusData,
    Pm2ProcessStatusDetail,
    Pm2ProcessStatusDetailData,
    startLedger,
    startMaster,
    startMd,
    startStrategy,
    startTd,
} from './processUtils';
import { Proc } from 'pm2';

interface SourceAccountId {
    source: string;
    id: string;
}

declare global {
    interface String {
        toAccountId(): string;
        parseSourceAccountId(): SourceAccountId;
        toSourceName(): string;
    }

    interface Array<T> {
        removeRepeat(): Array<T>;
        kfForEach(cb: <T>(t: T, index: number) => void): void;
        kfReverseForEach(cb: <T>(t: T, index: number) => void): void;
        kfForEachAsync(cb: <T>(t: T, index: number) => void): void;
    }
}

export {};

//因为accountid都是source_accountID,需要截取掉柜台名称
String.prototype.toAccountId = function (): string {
    if (this.indexOf('_') === -1) return this.toString();
    return this.split('_').slice(1).join('_');
};

String.prototype.toSourceName = function (): string {
    if (this.indexOf('_') === -1) return this.toString();
    return this.split('_')[0];
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
        console.log('<KF_INFO>', args.join(' '));
        logger.info('<KF_INFO>', args.join(' '));
    },

    warn: (...args: Array<string | number>) => {
        console.warn('<KF_INFO>', args.join(' '));
        logger.warn('<KF_INFO>', args.join(' '));
    },

    error: (...args: Array<string | number>) => {
        console.error('<KF_INFO>', args.join(' '));
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
        if (taskTimer) global.clearTimeout(taskTimer);
        fn().finally(() => {
            if (clear) {
                if (taskTimer) global.clearTimeout(taskTimer);
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
            if (taskTimer != null) global.clearTimeout(taskTimer);
        },
    };
};

export const loopToRunProcess = async (
    promiseFunc: Array<Function>,
    interval = 1000,
) => {
    let i = 0,
        len = promiseFunc.length;
    let resList = [];
    for (i = 0; i < len; i++) {
        const pFunc = promiseFunc[i];
        try {
            const res = await pFunc();
            resList.push(res);
        } catch (err) {
            resList.push(err);
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
    targetValue: string | number,
) => {
    const targetList = list.filter(
        (item) => (item || {})[targetKey] === targetValue,
    );
    if (targetList) {
        return targetList[0];
    }
    return null;
};

export const buildObjectFromArray = <T>(
    list: Array<T>,
    targetKey: number | string,
    targetValueKey?: number | string,
): { [prop: string | number]: T | T[keyof T] | undefined } => {
    let dict: { [prop: number | string]: T | T[keyof T] | undefined } = {};
    list.forEach((item) => {
        const key: number | string = (item || {})[targetKey] || '';
        if (key !== '' && key !== undefined) {
            if (targetValueKey === undefined) {
                dict[key] = item;
            } else {
                dict[key] = (item || {})[targetValueKey];
            }
        }
    });

    return dict;
};

export const getInstrumentTypeData = (
    instrumentType: InstrumentTypes,
): KfTradeValueCommonData => {
    return InstrumentType[
        InstrumentTypeEnum[instrumentType] || InstrumentTypeEnum.unknown
    ];
};

export const getCategoryData = (
    category: KfCategoryTypes,
): KfTradeValueCommonData => {
    if (KfCategory[KfCategoryEnum[category]]) {
        return KfCategory[KfCategoryEnum[category]];
    }

    throw new Error(`Category ${category} is illegal`);
};

const getChildFileStat = async (
    dirname: string,
): Promise<Array<{ childFilePath: string; stat: Stats }>> => {
    fse.ensureDirSync(dirname);
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
                    if (
                        fse.pathExistsSync(
                            path.join(childFilePath, 'package.json'),
                        )
                    ) {
                        extensionModuleDirs.push(childFilePath);
                    }
                } else {
                    const extModules = await flattenExtensionModuleDirs([
                        path.join(childFilePath, 'dist'),
                    ]);
                    extensionModuleDirs =
                        extensionModuleDirs.concat(extModules);
                }
            }
        }
    }

    return extensionModuleDirs;
};

const getKfExtConfigList = async (): Promise<KfExtOriginConfig[]> => {
    const extModuleDirs = await flattenExtensionModuleDirs([EXTENSION_DIR]);
    const packageJSONPaths = extModuleDirs.map((item) =>
        path.join(item, 'package.json'),
    );
    return await Promise.all(
        packageJSONPaths.map((item) => {
            return fse.readJSON(item);
        }),
    ).then((jsonList: { kungfuConfig?: KfExtOriginConfig }[]) => {
        return jsonList
            .map((json): KfExtOriginConfig | null => {
                return (json['kungfuConfig'] as KfExtOriginConfig) || null;
            })
            .filter(
                (config: KfExtOriginConfig | null): boolean => !!config,
            ) as KfExtOriginConfig[];
    });
};

const resolveInstrumentTypesInExtType = (
    types: InstrumentTypes | InstrumentTypes[],
): InstrumentTypes[] => {
    if (typeof types === 'string') {
        types = [types.toLowerCase() as InstrumentTypes];
        return types;
    }

    if (!types.length) {
        return ['unknown'];
    }

    return types.map((type) => type.toLowerCase()) as InstrumentTypes[];
};

const getKfExtensionConfigByCategory = (
    extConfigs: KfExtOriginConfig[],
): KfExtConfigs | Record<string, unknown> => {
    let configByCategory: KfExtConfigs = {};
    extConfigs.forEach((extConfig: KfExtOriginConfig) => {
        const extKey = extConfig.key;
        (Object.keys(extConfig['config']) as KfCategoryTypes[]).forEach(
            (category: KfCategoryTypes) => {
                if (configByCategory) {
                    const configOfCategory = extConfig['config'][category];
                    configByCategory[category] = {
                        ...(configByCategory[category] || {}),
                        [extKey]: {
                            type: resolveInstrumentTypesInExtType(
                                configOfCategory?.type || [],
                            ),
                            settings: configOfCategory?.settings || [],
                        },
                    };
                }
            },
        );
    });

    return configByCategory;
};

export const getKfExtensionConfig = async (): Promise<
    KfExtConfigs | Record<string, unknown>
> => {
    const kfExtConfigList = await getKfExtConfigList();
    return getKfExtensionConfigByCategory(kfExtConfigList);
};

export const buildExtTypeMap = (
    extConfigs: KfExtConfigs,
    category: KfCategoryTypes,
): Record<string, InstrumentTypes> => {
    const extTypeMap: Record<string, InstrumentTypes> = {};
    const targetCategoryConfig: Record<
        string,
        KfExtOriginConfig['config'][KfCategoryTypes]
    > = extConfigs[category] || {};

    Object.keys(targetCategoryConfig).forEach((extKey: string) => {
        const configInKfExtConfig = targetCategoryConfig[extKey];
        const types = resolveInstrumentTypesInExtType(
            configInKfExtConfig?.type || [],
        );

        if (!types.length) {
            extTypeMap[extKey] = 'unknown';
            return;
        }

        const primaryType = types.sort(
            (type1: InstrumentTypes, type2: InstrumentTypes) => {
                const level1 =
                    (
                        InstrumentType[
                            InstrumentTypeEnum[type1] ||
                                InstrumentTypeEnum.unknown
                        ] || {}
                    ).level || 0;
                const level2 =
                    (
                        InstrumentType[
                            InstrumentTypeEnum[type2] ||
                                InstrumentTypeEnum.unknown
                        ] || {}
                    ).level || 0;
                return level2 - level1;
            },
        )[0];

        extTypeMap[extKey] = primaryType;
    });

    return extTypeMap;
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

export const getSourceDataList = (
    extConfigs: KfExtConfigs,
    sourceType: KfCategoryTypes,
): SourceData[] => {
    const target = extConfigs[sourceType];
    return Object.keys(target || {})
        .map((extKey: string) => {
            const sourceInstrumentType = (target || {})[extKey]?.type;
            if (sourceInstrumentType === undefined) {
                return null;
            }

            const isTypeStr = typeof sourceInstrumentType === 'string';
            return {
                name: extKey,
                type: isTypeStr
                    ? [sourceInstrumentType]
                    : [...sourceInstrumentType],
            };
        })
        .filter(
            (sourceData: SourceData | null) => !!sourceData,
        ) as SourceData[];
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
        .set('hours', 15)
        .set('minutes', 30)
        .valueOf();

    if (currentTimestamp > tradingDayTimestamp) {
        return dayjs().add(1, 'day').format('YYYY-MM-DD');
    } else {
        return dayjs().format('YYYY-MM-DD');
    }
};

export const listDirSync = (filePath: string): string[] => {
    fse.ensureDirSync(filePath);
    return fse.readdirSync(filePath);
};

export const removeJournal = (targetFolder: string): Promise<void> => {
    function iterator(folder: string) {
        const items = listDirSync(folder);

        if (!items) return;

        const folders = items.filter((f: string) => {
            const stat = fse.statSync(path.join(folder, f));

            if (stat.isDirectory()) return true;
            return false;
        });

        const files = items.filter((f: string) => {
            const stat = fse.statSync(path.join(folder, f));

            if (stat.isFile()) return true;
            return false;
        });

        files.forEach((f: string) => {
            if (f.includes('.journal')) {
                fse.removeSync(path.join(folder, f));
            }
        });

        folders.forEach((f: string) => {
            iterator(path.join(folder, f));
        });
    }

    iterator(targetFolder);

    return Promise.resolve();
};

export const getProcessIdByKfLocation = (
    kfLocation: KfLocation | KfConfig,
): string => {
    if (kfLocation.category === 'td') {
        return `${kfLocation.category}_${kfLocation.group}_${kfLocation.name}`;
    } else if (kfLocation.category === 'md') {
        return `${kfLocation.category}_${kfLocation.group}`;
    } else if (kfLocation.category === 'strategy') {
        return `${kfLocation.category}_${kfLocation.name}`;
    } else if (kfLocation.category === 'system') {
        return kfLocation.name;
    }

    throw new Error(`Category ${kfLocation.category} is illegal`);
};

export const getIdByKfLocation = (
    kfLocation: KfLocation | KfConfig,
): string => {
    if (kfLocation.category === 'td') {
        return `${kfLocation.group}_${kfLocation.name}`;
    } else if (kfLocation.category === 'md') {
        return `${kfLocation.group}`;
    } else if (kfLocation.category === 'strategy') {
        return `${kfLocation.name}`;
    } else if (kfLocation.category === 'system') {
        return `${kfLocation.group}_${kfLocation.name}`;
    }

    throw new Error(`Category ${kfLocation.category} is illegal`);
};

export const getStateStatusData = (
    name: ProcessStatusTypes | undefined,
): KfTradeValueCommonData | undefined => {
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

export const getAppStateStatusName = (
    kfConfig: KfConfig | KfLocation,
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

//TOOD
export const getPropertyFromProcessStatusDetailDataByKfLocation = (
    processStatusDetailData: Pm2ProcessStatusDetailData,
    kfLocation: KfLocation | KfConfig,
): {
    status: ProcessStatusTypes | undefined;
    cpu: number;
    memory: string;
} => {
    const processStatusDetail: Pm2ProcessStatusDetail =
        processStatusDetailData[getProcessIdByKfLocation(kfLocation)] || {};
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

export const getConfigValue = (kfConfig: KfConfig) => {
    return JSON.parse(kfConfig.value || '{}');
};

export const buildIdByKeysFromKfConfigSettings = (
    kfConfigSetting: Record<string, KfConfigValue>,
    keys: string[],
) => {
    return keys
        .map((key) => kfConfigSetting[key])
        .filter((value) => value !== undefined)
        .join('_');
};

export const switchKfLocation = (
    watcher: Watcher | null,
    kfLocation: KfLocation | KfConfig,
    targetStatus: boolean,
): Promise<void | Proc> => {
    const processId = getProcessIdByKfLocation(kfLocation);

    if (!targetStatus) {
        if (kfLocation.category !== 'system') {
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
            }

        case 'td':
            return startTd(getIdByKfLocation(kfLocation));
        case 'md':
            return startMd(getIdByKfLocation(kfLocation));

        case 'strategy':
            const strategyPath =
                JSON.parse((kfLocation as KfConfig)?.value || '{}')
                    .strategy_path || '';
            if (!strategyPath) {
                throw new Error('Start Stratgy without strategy_path');
            }
            return startStrategy(getIdByKfLocation(kfLocation), strategyPath);
        default:
            return Promise.resolve();
    }
};

const dealKfNumber = (
    preNumber: bigint | number | undefined | unknown,
): string | number | bigint | unknown => {
    if (preNumber === undefined) return '--';
    return preNumber;
};

export const dealKfPrice = (
    preNumber: bigint | number | undefined | unknown,
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

export const dealSide = (side: SideEnum): KfTradeValueCommonData => {
    return Side[side];
};

export const dealOffset = (offset: OffsetEnum): KfTradeValueCommonData => {
    return Offset[offset];
};

export const dealDirection = (
    direction: DirectionEnum,
): KfTradeValueCommonData => {
    return Direction[direction];
};

export const dealOrderStatus = (
    status: OrderStatusEnum,
    errorMsg?: string,
): KfTradeValueCommonData => {
    return {
        ...OrderStatus[status],
        ...(+status === OrderStatusEnum.Error && errorMsg
            ? {
                  name: errorMsg,
              }
            : {}),
    };
};

export const dealInstrumentType = (instrumentType: InstrumentTypeEnum) => {
    return InstrumentType[instrumentType];
};

export const dealHedgeFlag = (
    hedgeFlag: HedgeFlagEnum,
): KfTradeValueCommonData => {
    return HedgeFlag[hedgeFlag];
};

export const dealCategory = (
    category: KfCategoryTypes,
): KfTradeValueCommonData => {
    return KfCategory[KfCategoryEnum[category]];
};

export const dealLocationUID = (
    watcher: Watcher | null,
    uid: number,
): string => {
    if (!watcher) {
        return '--';
    }

    const kfLocation = watcher?.getLocation(uid);
    return getIdByKfLocation(kfLocation);
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
    typename: TradingDataTypeName,
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
        //TODO  instruments
        return LedgerCategoryEnum['td'];
    }

    return LedgerCategoryEnum[category as LedgerCategoryTypes];
};

export const filterLedgerResult = <T>(
    dataTable: DataTable<T>,
    tradingDataTypeName: TradingDataTypeName,
    kfLocation: KfLocation | KfConfig,
    sortKey?: string,
): T[] => {
    const { category, group, name } = kfLocation;
    const ledgerCategory = getLedgerCategory(category);
    let dataTableResolved = dataTable.filter('ledger_category', ledgerCategory);

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

export const dealTradingData = (
    watcher: Watcher | null,
    tradingData: TradingData,
    tradingDataTypeName: TradingDataTypeName,
    kfLocation: KfLocation | KfConfig,
): TradingDataNameToType[TradingDataTypeName][] => {
    if (!watcher) {
        throw new Error('dealTradingData no watcher');
    }

    const currentUID = watcher.getLocationUID(kfLocation);
    const orderTradeFilterKey = getOrderTradeFilterKey(kfLocation.category);
    const sortKey = getTradingDataSortKey(tradingDataTypeName);

    if (
        tradingDataTypeName === 'Order' ||
        tradingDataTypeName === 'Trade' ||
        tradingDataTypeName === 'OrderInput'
    ) {
        const historyDatas = tradingData[tradingDataTypeName].filter(
            orderTradeFilterKey,
            currentUID,
        );

        if (sortKey) {
            return historyDatas.sort(sortKey);
        } else {
            return historyDatas.list();
        }
    }

    return filterLedgerResult<TradingDataNameToType[TradingDataTypeName]>(
        tradingData[tradingDataTypeName],
        tradingDataTypeName,
        kfLocation,
        sortKey,
    );
};
