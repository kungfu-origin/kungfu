import { KfCategory } from '../config/tradingConfig';
import { KfLocation } from '../typings';

export const getProcessIdByKfLocation = (kfLocation: KfLocation): string => {
    if (kfLocation.category === 'td') {
        return `${kfLocation.category}_${kfLocation.group}_${kfLocation.name}`;
    } else if (kfLocation.category === 'md') {
        return `${kfLocation.category}_${kfLocation.group}`;
    } else if (kfLocation.category === 'strategy') {
        return `${kfLocation.category}_${kfLocation.name}`;
    } else if (kfLocation.category === 'system') {
        return kfLocation.name;
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
    if (KfCategory[kfLocation.category]) {
        return KfCategory[kfLocation.category].name;
    }

    throw new Error(`Category ${kfLocation.category} is illegal`);
};
