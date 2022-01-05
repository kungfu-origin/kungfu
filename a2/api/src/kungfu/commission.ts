import { kfLogger } from '../utils/busiUtils';
import { commissionStore, longfist } from './index';

export const getKfCommission = (): Promise<KungfuApi.Commission[]> => {
    kfLogger.info('Get kungfu Commission');
    return new Promise((resolve, reject) => {
        try {
            const commissionData = commissionStore.getAllCommission();
            resolve(
                commissionData.sort((a, b) =>
                    a.exchange_id.localeCompare(b.exchange_id),
                ),
            );
        } catch (err) {
            reject(err);
        }
    });
};

export const setKfCommission = (commissions: KungfuApi.Commission[]) => {
    kfLogger.info('Set kungfu Commission');
    return new Promise((resolve, reject) => {
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

        const result = commissionStore.setAllCommission(comissionsResolved);

        if (result) {
            resolve(result);
        } else {
            reject(result);
        }
    });
};
