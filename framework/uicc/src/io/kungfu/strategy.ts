import { getAllKfConfig, setKfConfig, getKfConfig, removeKfConfig } from '@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils';

export const getStrategyList = (): Promise<Strategy[]> => {
    return new Promise(resolve => {
        const kfConfigList = getAllKfConfig();
        const strategyList = Object.values(kfConfigList || {})
            .filter((strategyData: any) => {
                return +strategyData.category === 2
            })
            .map((strategyData: any) => {
                return {
                    ...JSON.parse(strategyData.value || '{}')
                }
            })
            .sort((strategyData1: any, strategyData2: any) => {
                return strategyData1.strategy_id.localeCompare(strategyData2.strategy_id);
            })
                    
        resolve(strategyList)
    })
}

export const getStrategyById = (strategyId: string): Promise<Array<Strategy>> => {
    return new Promise(resolve => {
        const strategyData: any = getKfConfig(strategyId, 'strategy')
        const strategy: any[] = [{ ...JSON.parse(strategyData.value || '{}') }]
        resolve(strategy)
    })
}


export const addStrategy = (strategyId: string, strategyPath: string) => {
    const addTime = +new Date().getTime() * Math.pow(10,6)
    return new Promise(resolve => {
        const strategy = setKfConfig(strategyId, 'strategy', JSON.stringify({
            strategy_id: strategyId,
            strategy_path: strategyPath,
            add_time: addTime
        }));
        resolve(strategy)
    })
}

export const deleteStrategy = (strategyId: string) => {
    return new Promise(resolve => {
        const strategy = removeKfConfig(strategyId, 'strategy');
        resolve(strategy)
    })
}
 
export const updateStrategyPath = async (strategyId: string, strategyPath: string) => {
    let addTime = +new Date().getTime() * Math.pow(10,6)
    const strategyOld: Array<Strategy> = await getStrategyById(strategyId)
    if (strategyOld.length) {
        addTime = strategyOld[0].add_time
    }
    return new Promise(resolve => {
        const strategy = setKfConfig(strategyId, 'strategy', JSON.stringify({
            strategy_id: strategyId,
            strategy_path: strategyPath,
            add_time: addTime
        }));
        resolve(strategy)
    })
}