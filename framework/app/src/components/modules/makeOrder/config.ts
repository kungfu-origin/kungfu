import { ShotableInstrumentTypes } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
    InstrumentTypeEnum,
    KfCategoryTypes,
    PriceTypeEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';

export const getConfigSettings = (
    category?: KfCategoryTypes,
    instrumentTypeEnum?: InstrumentTypeEnum,
    priceType?: PriceTypeEnum,
): KungfuApi.KfConfigItem[] => {
    const shotable = instrumentTypeEnum
        ? ShotableInstrumentTypes.includes(
              instrumentTypeEnum || InstrumentTypeEnum.unknown,
          )
        : false;

    const defaultSettings: KungfuApi.KfConfigItem[] = [
        category === 'td'
            ? null
            : {
                  key: 'account_id',
                  name: '账户',
                  type: 'td',
                  required: true,
              },
        {
            key: 'instrument',
            name: '标的',
            type: 'instrument',
            required: true,
        },
        {
            key: 'side',
            name: '买卖',
            type: 'side',
            default: 0,
            required: true,
        },
        ...(shotable
            ? [
                  {
                      key: 'offset',
                      name: '开平',
                      type: 'offset',
                      default: 0,
                      required: true,
                  },
                  {
                      key: 'hedge_flag',
                      name: '套保',
                      type: 'hedgeFlag',
                      default: 0,
                      required: true,
                  },
              ]
            : []),
        {
            key: 'price_type',
            name: '方式',
            type: 'priceType',
            default: 0,
            required: true,
        },
        {
            key: 'price',
            name: priceType === PriceTypeEnum.Limit ? '价格' : '保护价格',
            type: 'float',
            required: priceType === PriceTypeEnum.Limit ? true : false,
        },
        {
            key: 'volume',
            name: '下单量',
            type: 'int',
            required: true,
        },
    ].filter((item) => !!item) as KungfuApi.KfConfigItem[];

    return defaultSettings;
};
