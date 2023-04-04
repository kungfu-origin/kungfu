import { kfLogger, getResultUntilValuable } from '../utils/busiUtils';
import { commissionStore, longfist } from './index';

export const getKfCommission = (): Promise<KungfuApi.Commission[]> => {
  kfLogger.info('Get kungfu Commission');
  return getResultUntilValuable(() => commissionStore.getAllCommission()).then(
    (allCommissions) =>
      allCommissions.sort((a, b) => a.exchange_id.localeCompare(b.exchange_id)),
  );
};

export const setKfCommission = (
  commissions: KungfuApi.Commission[],
): Promise<boolean> => {
  kfLogger.info('Set kungfu Commission');
  const kfCommissionData = longfist.Commission();
  const comissionsResolved = commissions
    .filter((item) => {
      return item.product_id && item.exchange_id;
    })
    .map((item: KungfuApi.Commission) => {
      return {
        ...kfCommissionData,
        ...item,
      };
    });

  return getResultUntilValuable(() =>
    commissionStore.setAllCommission(comissionsResolved),
  );
};
