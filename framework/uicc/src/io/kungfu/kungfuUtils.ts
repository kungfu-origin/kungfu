import moment from 'moment';
import { KF_RUNTIME_DIR } from '@kungfu-trader/kungfu-uicc/config/pathConfig';
import { logger } from '@kungfu-trader/kungfu-uicc/utils/logUtils';

export const kungfu = require('@kungfu-trader/kungfu-core').kungfu;
export const longfist = kungfu.longfist;
export const kungfuConfigStore = kungfu.ConfigStore(KF_RUNTIME_DIR);
export const commissionStore = kungfu.CommissionStore(KF_RUNTIME_DIR);
export const history = kungfu.History(KF_RUNTIME_DIR);

export const getAllKfConfig = () => {
    logger.info("Get All Kungfu Config")
    console.log("Get All Kungfu Config")
    return kungfuConfigStore.getAllConfig();
}

export const setKfConfig = (key: string, type: string, config: string) => {
    logger.info("Set Kungfu Config", key, type, config);
    console.log("Set Kungfu Config", key, type, config);
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.setConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode, config)
}

export const getKfConfig = (key: string, type: string) => {
    logger.info("Get Kungfu Config", key, type);
    console.log("Get Kungfu Config", key, type);
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.getConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode)
}

export const removeKfConfig = (key: string, type: string) => {
    logger.info("Remove Kungfu Config", key, type);
    console.log("Remove Kungfu Config", key, type);
    const kungfuKey = encodeKungfuLocation(key, type);
    return kungfuConfigStore.removeConfig(kungfuKey.category, kungfuKey.group, kungfuKey.name, kungfuKey.mode)
}

export const getKfCommission = () => {
    logger.info("Get kungfu Commission");
    console.log("Get kungfu Commission");
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
    logger.info("Set kungfu Commission");
    console.log("Set kungfu Commission");
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


export function getKungfuDataByDateRange (date: number | string, dateType: number) {
    const tradingDay = moment(date).format("YYYYMMDD");
    const from = moment(date).format('YYYY-MM-DD');
    const to = moment(date).add(1, 'day').format('YYYY-MM-DD');
    const yesFrom = moment(date).add(-1, 'day').format('YYYY-MM-DD');
    const fridayFrom = moment(date).add(-3, 'day').format('YYYY-MM-DD');
    const fridayTo = moment(date).add(-2, 'day').format('YYYY-MM-DD');
    const dataTypeForHistory = ["Order", "Trade", "OrderStat", "Position"];
    return new Promise(resolve => {
        let timer = setTimeout(() => {

            //by trading date
            if (dateType === 1) {
                const kungfuDataToday = history.selectPeriod(from, to);
                const kungfuDataYesterday = history.selectPeriod(yesFrom, from);
                const kungfuDatafriday = history.selectPeriod(fridayFrom, fridayTo);
                let historyData: any = {};
                dataTypeForHistory.forEach(key => {
    
                    if (key === "Order" || key === "Trade") {
                        historyData[key] = Object.assign(
                            kungfuDataToday[key].filter("trading_day", tradingDay),
                            kungfuDataYesterday[key].filter("trading_day", tradingDay),
                            kungfuDatafriday[key].filter("trading_day", tradingDay)
                        )
                    } else {
                        historyData[key] = Object.assign(
                            kungfuDatafriday[key],
                            kungfuDataYesterday[key],
                            kungfuDataToday[key],
                        )
                    }
                })
    
                resolve(historyData)
            } else {
                const kungfuDataToday = history.selectPeriod(from, to);
                resolve(kungfuDataToday)
            }
            
            clearTimeout(timer);
        }, 100)
    })
}