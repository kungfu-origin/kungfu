import { watcher } from '__gUtils/kungfuUtils';

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

export const decodeKungfuLocation = (source: string) => {
    const location: KungfuLocation = watcher.getLocation(source)
    return location
}