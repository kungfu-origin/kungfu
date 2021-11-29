import path from 'path';
import log4js from 'log4js';
import { buildProcessLogPath } from '@kungfu-trader/kungfu-js-api/config/pathConfig';

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
    var taskTimer: null | NodeJS.Timeout = null;
    var clear = false;
    function timerPromiseTask(fn: Function, interval = 500) {
        if (taskTimer) global.clearTimeout(taskTimer);
        fn().finally(() => {
            if (clear) {
                if (taskTimer) global.clearTimeout(taskTimer);
                return;
            }
            taskTimer = global.setTimeout(() => {
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
