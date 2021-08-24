// @ts-ignore
import { KF_RUNTIME_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import moment from 'moment';

export const kungfu = require('@kungfu-trader/kungfu-core').kungfu;
export const longfist = kungfu.longfist;
export const kungfuConfigStore = kungfu.ConfigStore(KF_RUNTIME_DIR);
export const commissionStore = kungfu.CommissionStore(KF_RUNTIME_DIR);
export const history = kungfu.History(KF_RUNTIME_DIR);

export const getAllKfConfig = () => {
    return kungfuConfigStore.getAllConfig();
}

export const setKfConfig = (key: string, type: string, config: string) => {
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.setConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode, config)
}

export const getKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.getConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode)
}

export const removeKfConfig = (key: string, type: string) => {
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.removeConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode)
}

export const getKfCommission = () => {
    return new Promise((resolve, reject) => {
        try {
            const commissionData = commissionStore.getAllCommission();
            resolve(Object.values(commissionData || {}))
        } catch (err) {
            reject(err)
        }
    })
}

export const setKfCommission = (commissionItems: any) => {
    return new Promise((resolve, reject) => {
        const kfCommissionData = longfist.Commission()
        const comissionResolve = commissionItems.map((item: any) => {
            return {
                ...kfCommissionData,
                ...item
            }
        })

        const result = commissionStore.setAllCommission(comissionResolve)

        if (result) {
            resolve(result)
        } else {
            reject(result)
        }
    })

}


export function encodeKungfuLocation(key: string, type: string): KungfuLocation {
    switch (type) {
        case 'td':
            const tdIdSplit = key.split('_');
            if (tdIdSplit.length !== 2) throw new Error(`td id ${key} is illegal!`);
            const sourceId = tdIdSplit[0];
            const accountId = tdIdSplit[1];
            ['td', sourceId, accountId, 'live']
            return {
                category: 'td',
                group: sourceId,
                name: accountId,
                mode: 'live'
            };
        case 'md':
            if (!key) throw new Error(`md id ${key} is illegal!`);
            return {
                category: 'md',
                group: key,
                name: key,
                mode: 'live'
            }
        case 'strategy':
            if (!key) throw new Error(`strategy id ${key} is illegal!`);
            return {
                category: 'strategy',
                group: 'default',
                name: key,
                mode: 'live'
            }
        default:
            throw new Error(`unknow type ${type}`);
    }
}


export function getKungfuDataByDateRange (date: number | string) {
    const from = moment(date).format('YYYY-MM-DD');
    const to = moment(date).add(1, 'day').format('YYYY-MM-DD');
    return new Promise(resolve => {
        let timer = setTimeout(() => {
            const kungfuData = history.selectPeriod(from, to);
            resolve(kungfuData)
            clearTimeout(timer);
        }, 100)
    })
}