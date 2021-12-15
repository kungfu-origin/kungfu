import path from 'path';
import dayjs from 'dayjs';
import fse from 'fs-extra';
import log4js from 'log4js';
import { buildProcessLogPath } from '../config/pathConfig';
import {
    InstrumentType,
    KfCategory,
    StateStatus,
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
} from '../typings';
import {
    Pm2ProcessStatusData,
    Pm2ProcessStatusDetail,
    Pm2ProcessStatusDetailData,
} from './processUtils';

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
    instrumentType: InstrumentTypeEnum,
): KfTradeValueCommonData => {
    return InstrumentType[instrumentType];
};

export const getKfExtensionConfig = (): Promise<
    KfExtConfigs | Record<string, unknown>
> => {
    const getExtConfigList = (): Promise<KfExtOriginConfig[]> =>
        Promise.resolve([
            {
                key: 'sim',
                name: 'SIM',
                config: {
                    td: {
                        type: 'Simu',
                        settings: [
                            {
                                key: 'str',
                                name: 'str',
                                type: 'str',
                                errMsg: '请填写 str',
                                required: true,
                                primary: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'int',
                                name: 'int',
                                type: 'int',
                                errMsg: '请填写 int',
                                required: true,
                                primary: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'float',
                                name: 'float',
                                type: 'float',
                                errMsg: '请填写 float',
                                required: true,
                                primary: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'percent',
                                name: 'percent',
                                type: 'percent',
                                errMsg: '请填写 percent',
                                required: true,
                                primary: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'side',
                                name: 'side',
                                type: 'side',
                                errMsg: '请填写 side',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'offset',
                                name: 'offset',
                                type: 'offset',
                                errMsg: '请填写 offset',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'direction',
                                name: 'direction',
                                type: 'direction',
                                errMsg: '请填写 direction',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'priceType',
                                name: 'priceType',
                                type: 'priceType',
                                errMsg: '请填写 priceType',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },

                            {
                                key: 'hedgeFlag',
                                name: 'hedgeFlag',
                                type: 'hedgeFlag',
                                errMsg: '请填写 hedgeFlag',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'volumeCondition',
                                name: 'volumeCondition',
                                type: 'volumeCondition',
                                errMsg: '请填写 volumeCondition',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'timeCondition',
                                name: 'timeCondition',
                                type: 'timeCondition',
                                errMsg: '请填写 timeCondition',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'commissionMode',
                                name: 'commissionMode',
                                type: 'commissionMode',
                                errMsg: '请填写 commissionMode',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'instrumentType',
                                name: 'instrumentType',
                                type: 'instrumentType',
                                errMsg: '请填写 instrumentType',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'password',
                                name: 'password',
                                type: 'password',
                                errMsg: '请填写 password',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'match_mode',
                                name: 'match_mode',
                                type: 'select',
                                options: [
                                    {
                                        value: 'reject',
                                        label: 'reject',
                                    },
                                    {
                                        value: 'pend',
                                        label: 'pend',
                                    },
                                    {
                                        value: 'cancel',
                                        label: 'cancel',
                                    },
                                    {
                                        value: 'partialfillandcancel',
                                        label: 'partialfillandcancel',
                                    },
                                    {
                                        value: 'partialfill',
                                        label: 'partialfill',
                                    },
                                    {
                                        value: 'fill',
                                        label: 'fill',
                                    },
                                ],
                                errMsg: '请选择撮合模式',
                                required: true,
                            },
                            {
                                key: 'exchange',
                                name: 'exchange',
                                type: 'exchange',
                                errMsg: '请填写 exchange',
                                required: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'file',
                                name: 'file',
                                type: 'file',
                                errMsg: '请填写 file',
                                required: true,
                            },
                        ],
                    },
                },
            },
            {
                key: 'xtp',
                name: 'XTP',
                config: {
                    td: {
                        type: ['Stock', 'TechStock', 'Bond'],
                        settings: [
                            {
                                key: 'account_id',
                                name: 'account_id',
                                type: 'str',
                                errMsg: '请填写账户 account_id',
                                required: true,
                                primary: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'match_mode',
                                name: 'match_mode',
                                type: 'select',
                                options: [
                                    {
                                        value: 'reject',
                                        label: 'reject',
                                    },
                                    {
                                        value: 'pend',
                                        label: 'pend',
                                    },
                                    {
                                        value: 'cancel',
                                        label: 'cancel',
                                    },
                                    {
                                        value: 'partialfillandcancel',
                                        label: 'partialfillandcancel',
                                    },
                                    {
                                        value: 'partialfill',
                                        label: 'partialfill',
                                    },
                                    {
                                        value: 'fill',
                                        label: 'fill',
                                    },
                                ],
                                errMsg: '请选择撮合模式',
                                required: true,
                            },
                        ],
                    },
                },
            },
            {
                key: 'ctp',
                name: 'CTP',
                config: {
                    td: {
                        type: ['Future'],
                        settings: [
                            {
                                key: 'account_id',
                                name: 'account_id',
                                type: 'str',
                                errMsg: '请填写账户 account_id',
                                required: true,
                                primary: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'match_mode',
                                name: 'match_mode',
                                type: 'select',
                                options: [
                                    {
                                        value: 'reject',
                                        label: 'reject',
                                    },
                                    {
                                        value: 'pend',
                                        label: 'pend',
                                    },
                                    {
                                        value: 'cancel',
                                        label: 'cancel',
                                    },
                                    {
                                        value: 'partialfillandcancel',
                                        label: 'partialfillandcancel',
                                    },
                                    {
                                        value: 'partialfill',
                                        label: 'partialfill',
                                    },
                                    {
                                        value: 'fill',
                                        label: 'fill',
                                    },
                                ],
                                errMsg: '请选择撮合模式',
                                required: true,
                            },
                        ],
                    },
                    md: {
                        type: ['Future'],
                        settings: [
                            {
                                key: 'account_id',
                                name: 'account_id',
                                type: 'str',
                                errMsg: '请填写账户 account_id',
                                required: true,
                                primary: true,
                                validator: ['noUnderlineValidator'],
                            },
                            {
                                key: 'match_mode',
                                name: 'match_mode',
                                type: 'select',
                                options: [
                                    {
                                        value: 'reject',
                                        label: 'reject',
                                    },
                                    {
                                        value: 'pend',
                                        label: 'pend',
                                    },
                                    {
                                        value: 'cancel',
                                        label: 'cancel',
                                    },
                                    {
                                        value: 'partialfillandcancel',
                                        label: 'partialfillandcancel',
                                    },
                                    {
                                        value: 'partialfill',
                                        label: 'partialfill',
                                    },
                                    {
                                        value: 'fill',
                                        label: 'fill',
                                    },
                                ],
                                errMsg: '请选择撮合模式',
                                required: true,
                            },
                        ],
                    },
                },
            },
        ]);

    return getExtConfigList().then((kfExtConfigList: KfExtOriginConfig[]) => {
        return getKfExtensionConfigByCategory(kfExtConfigList);
    });
};

function getKfExtensionConfigByCategory(
    extConfigs: KfExtOriginConfig[],
): KfExtConfigs | Record<string, unknown> {
    let configByCategory: KfExtConfigs | null = {};
    extConfigs.forEach((extConfig: KfExtOriginConfig) => {
        const extKey = extConfig.key;
        (Object.keys(extConfig['config']) as KfCategoryTypes[]).forEach(
            (category: KfCategoryTypes) => {
                if (configByCategory) {
                    configByCategory[category] = {
                        ...(configByCategory[category] || {}),
                        [extKey]: extConfig['config'][category],
                    };
                }
            },
        );
    });

    return configByCategory;
}

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
    }

    throw new Error(`Category ${kfLocation.category.toString()} is illegal`);
};

export const getCategoryName = (kfLocation: KfLocation | KfConfig): string => {
    if (KfCategory[kfLocation.category]) {
        return KfCategory[kfLocation.category].name;
    }

    throw new Error(`Category ${kfLocation.category} is illegal`);
};

export const getStateStatusData = (
    name: ProcessStatusTypes | undefined,
): KfTradeValueCommonData | undefined => {
    return name === undefined ? StateStatus['Unknown'] : StateStatus[name];
};

export const getIfProcessOnline = (
    processStatusData: Pm2ProcessStatusData,
    processId: string,
): boolean => {
    if (processStatusData[processId]) {
        if (processStatusData[processId] === 'online') {
            return true;
        }
    }

    return false;
};

export const getPropertyFromProcessStatusDetailDataByKfLocation = (
    processStatusDetailData: Pm2ProcessStatusDetailData,
    kfLocation: KfLocation | KfConfig,
): {
    status: ProcessStatusTypes;
    cpu: number;
    memory: string;
} => {
    const processStatusDetail: Pm2ProcessStatusDetail =
        processStatusDetailData[getProcessIdByKfLocation(kfLocation)] || {};
    const status = processStatusDetail.status || 'Unknown';
    const monit = processStatusDetail.monit || {};

    return {
        status,
        cpu: monit.cpu || 0,
        memory: Number((monit.memory || 0) / (1024 * 1024)).toFixed(2),
    };
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
        let types = configInKfExtConfig?.type || [];
        if (typeof types === 'string') {
            types = [types];
        }

        if (!types.length) {
            extTypeMap[extKey] = 'Unknown';
            return;
        }

        const primaryType = types.sort(
            (type1: InstrumentTypes, type2: InstrumentTypes) => {
                const level1 =
                    (InstrumentType[InstrumentTypeEnum[type1]] || {}).level ||
                    0;
                const level2 =
                    (InstrumentType[InstrumentTypeEnum[type2]] || {}).level ||
                    0;
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
