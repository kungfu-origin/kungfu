import { setKfConfig } from './store';
import { configStore } from '@kungfu-trader/kungfu-js-api/kungfu/index'

export const getStrategyById = (
  strategyId: string,
): Promise<Array<Code.Strategy>> => {
  return new Promise((resolve) => {
    const strategyData: any = getKfConfig(strategyId);
    const strategy: Array<Code.Strategy> = [{ ...JSON.parse(strategyData.value || '{}') }];
    resolve(strategy);
  });
};

export const getKfConfig = (strategyId: string) => {
  const kfLocation: KungfuApi.KfLocation = getStrategyKfLocation(strategyId);
  return configStore.getConfig(
      kfLocation.category,
      kfLocation.group,
      kfLocation.name,
      kfLocation.mode
  );
};

export const updateStrategyPath = async (
  strategyId: string,
  strategyPath: string,
) => {
  let addTime = +new Date().getTime() * Math.pow(10, 6);
  const strategyOld: Array<Code.Strategy> = await getStrategyById(strategyId);
  const kfLocation = getStrategyKfLocation(strategyId)
  if (strategyOld.length) {
    addTime = strategyOld[0].add_time;
  }
  return new Promise((resolve) => {
    const strategy = setKfConfig(
      kfLocation,
      JSON.stringify({
        strategy_id: strategyId,
        strategy_path: strategyPath,
        add_time: addTime,
      }),
    );
    resolve(strategy);
  });
};

export const getStrategyKfLocation = (strategyId: string) => {
    return {
        category: 'strategy',
        group: 'default',
        name: strategyId,
        mode: 'live',
    }
}