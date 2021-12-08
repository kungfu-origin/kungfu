import {
    KfLocation,
} from '@kungfu-trader/kungfu-js-api/typings';

export const getProcessIdByKfLocation = (kfLocation: KfLocation): string => {
    if (kfLocation.category === 'td') {
        return `${kfLocation.category}_${kfLocation.group}_${kfLocation.name}`;
    } else if (kfLocation.category === 'md') {
        return `${kfLocation.category}_${kfLocation.group}`;
    } else if (kfLocation.category === 'strategy') {
        return `${kfLocation.category}_${kfLocation.name}`;
    }

    throw new Error(`Category ${kfLocation.category} is illegal`);
};

export const getIdByKfLocation = (kfLocation: KfLocation): string => {
    if (kfLocation.category === 'td') {
        return `${kfLocation.group}_${kfLocation.name}`;
    } else if (kfLocation.category === 'md') {
        return `${kfLocation.group}`;
    } else if (kfLocation.category === 'strategy') {
        return `${kfLocation.name}`;
    }

    throw new Error(`Category ${kfLocation.category} is illegal`);
};

export const getCategoryName = (kfLocation: KfLocation): string => {
    if (kfLocation.category === 'td') {
        return '交易账户';
    } else if (kfLocation.category === 'md') {
        return '行情源';
    } else if (kfLocation.category === 'strategy') {
        return '策略';
    }

    throw new Error(`Category ${kfLocation.category} is illegal`);
};
