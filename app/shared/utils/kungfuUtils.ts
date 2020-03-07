
import { KF_HOME } from '__gConfig/pathConfig';
import { setTimerPromiseTask } from '__gUtils/BusiUtils';

process.env.KF_LOG_LEVEL = 'trace';

export const kungfu = require('kungfu-core').kungfu;
export const watcher = kungfu.watcher(KF_HOME, `watcher_${process.env.APP_TYPE}`);
export const longfist = kungfu.longfist;
export const startGetKungfuState = (callback: Function, interval = 1000) => {
    setTimerPromiseTask(() => {
        return new Promise((resolve) => {
            if (!watcher.isLive() && !watcher.isStarted() && watcher.isUsable()) {
                watcher.setup();
            }
            
            if (watcher.isLive()) {
                watcher.step();
            }
            
            callback(Object.freeze(watcher.ledger));
            resolve();
        })
    }, interval);
}

const kungfuConfigStore = kungfu.ConfigStore(KF_HOME);

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


export const transformOrderTradeListToData = (list: any[], type: string) => {
    let data: StringToAny = {};

    if (type === 'account') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.source)
            const accountId = `${location.group}_${location.name}`;
            if (!accountId) return;
            if (!data[accountId]) data[accountId] = [];
            data[accountId].push(item)
        })
    } else if (type === 'strategy') {
        list.kfForEach((item: any) => {
            const location = decodeKungfuLocation(item.dest)
            const clientId = location.name;
            if (!clientId) return;
            if (!data[clientId]) data[clientId] = [];
            data[clientId].push(item)
        })
    }
    return data;
}

export const transformTradingItemListToData = (list: any[], type: string) => {
    let data: StringToAny = {}

    if (type === 'account') {
        list.kfForEach((item: any) => {
            const accountId = `${item.source_id}_${item.account_id}`;
            if (!accountId) return;
            const ledgerCategory = +item.ledger_category;
            if (ledgerCategory === 0) {
                if (!data[accountId]) data[accountId] = [];
                data[accountId].push(item)
            }
        })
    } else if (type === 'strategy') {
        list.kfForEach((item: any) => {
            const clientId = item.client_id;
            if (!clientId) return;
            const ledgerCategory = +item.ledger_category;
            if (ledgerCategory === 1) {
                if (!data[clientId]) data[clientId] = [];
                data[clientId].push(item)
            }
        })
    }

    return data
}

export const transformAssetItemListToData = (list: any[], type: string) => {
    let accountIdClientIdData = transformTradingItemListToData(list, type);
    Object.keys(accountIdClientIdData || {}).forEach((id: string) => {
        const valueData = accountIdClientIdData[id]
            .reduce((a: any, b: any) => {
                return {
                    ...a,
                    ...b
                }
            })
        accountIdClientIdData[id] = valueData
    })
    return accountIdClientIdData
}

export function decodeKungfuLocation (source: string) {
    const location: KungfuLocation = watcher.getLocation(source)
    return location
}


export function encodeKungfuLocation (key: string, type: string): KungfuLocation {
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
            } ;
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


