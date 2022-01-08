import {
    HedgeFlagEnum,
    KfCategoryTypes,
    OffsetEnum,
    PriceTypeEnum,
    SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';

export const getConfigSettings = (
    category?: KfCategoryTypes,
    side?: SideEnum | undefined,
): KungfuApi.KfConfigItem[] =>
    [
        category === 'td'
            ? null
            : {
                  key: 'account_id',
                  name: '账户',
                  type: 'td',
                  required: true,
              },
        {
            key: 'future_arbitrage_code',
            name: '套利代码',
            type: 'futureArbitrageCode',
            required: true,
        },
        {
            key: 'instrument_A',
            name: side === SideEnum.Buy ? '买入标的 A' : '卖出标的 A',
            type: 'instrument',
            required: true,
        },
        {
            key: 'instrument_B',
            name: side === SideEnum.Buy ? '卖出标的 B' : '买入标的 B',
            type: 'instrument',
            required: true,
        },
        {
            key: 'side',
            name: '买卖',
            type: 'side',
            default: SideEnum.Buy,
            required: true,
        },
        {
            key: 'offset',
            name: '开平',
            type: 'offset',
            default: OffsetEnum.Open,
            required: true,
        },
        {
            key: 'hedge_flag',
            name: '套保',
            type: 'hedgeFlag',
            default: HedgeFlagEnum.Speculation,
            required: true,
        },
        {
            key: 'price_type',
            name: '方式',
            type: 'priceType',
            default: PriceTypeEnum.Limit,
            required: true,
        },
        {
            key: 'limit_price',
            name: '价差 A-B',
            type: 'float',
            required: true,
        },
        {
            key: 'volume',
            name: '下单量',
            type: 'int',
            required: true,
        },
    ].filter((item) => !!item) as KungfuApi.KfConfigItem[];
