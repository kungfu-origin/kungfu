import {
    KfLocation,
    KfCategoryTypes,
} from '@kungfu-trader/kungfu-js-api/typings';

export function toKfLocation(
    name: string, // accountName: ${sourceId}_${accountId}, sourceName: ${sourceId}, strategyName: strategyId
    type: KfCategoryTypes,
): KfLocation {
    switch (type) {
        case 'td':
            const tdIdSplit = name.split('_');
            if (tdIdSplit.length !== 2) {
                throw new Error(`td id ${name} is illegal!`);
            }
            const sourceId = tdIdSplit[0];
            const accountId = tdIdSplit[1];
            return {
                category: 'td',
                group: sourceId,
                name: accountId,
                mode: 'live',
            };
        case 'md':
            if (!name) throw new Error(`md id ${name} is illegal!`);
            return {
                category: 'md',
                group: name,
                name: name,
                mode: 'live',
            };
        case 'strategy':
            if (!name) throw new Error(`strategy id ${name} is illegal!`);
            return {
                category: 'strategy',
                group: 'default',
                name: name,
                mode: 'live',
            };
        default:
            throw new Error(`unknow type ${type}`);
    }
}
