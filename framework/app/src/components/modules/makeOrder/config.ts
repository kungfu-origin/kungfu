import { ShotableInstrumentTypes } from '@kungfu-trader/kungfu-js-api/config/tradingConfig';
import {
  HedgeFlagEnum,
  InstrumentTypeEnum,
  KfCategoryTypes,
  OffsetEnum,
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

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
          name: t('tradingConfig.account'),
          type: 'td',
          required: true,
        },
    {
      key: 'instrument',
      name: t('tradingConfig.instrument'),
      type: 'instrument',
      required: true,
    },
    {
      key: 'side',
      name: t('tradingConfig.side'),
      type: 'side',
      default: SideEnum.Buy,
      required: true,
    },
    ...(shotable
      ? [
          {
            key: 'offset',
            name: t('tradingConfig.offset'),
            type: 'offset',
            default: OffsetEnum.Open,
            required: true,
          },
          {
            key: 'hedge_flag',
            name: t('tradingConfig.hedge'),
            type: 'hedgeFlag',
            default: HedgeFlagEnum.Speculation,
            required: true,
          },
        ]
      : []),
    {
      key: 'price_type',
      name: t('tradingConfig.price_type'),
      type: 'priceType',
      default: PriceTypeEnum.Limit,
      required: true,
    },
    {
      key: 'limit_price',
      name:
        priceType === PriceTypeEnum.Limit
          ? t('tradingConfig.price')
          : t('tradingConfig.protect_price'),
      type: 'float',
      required: priceType === PriceTypeEnum.Limit ? true : false,
    },
    {
      key: 'volume',
      name: t('tradingConfig.volume'),
      type: 'int',
      required: true,
    },
  ].filter((item) => !!item) as KungfuApi.KfConfigItem[];

  return defaultSettings;
};

export const makeOrderConfigKFTypes = [
  'td',
  'instrument',
  'side',
  'offset',
  'hedgeFlag',
  'pricetype',
];

export const orderInputTrans: Record<string, string> = {
  account_id: t('tradingConfig.account'),
  instrument_id: `${t('tradingConfig.instrument')}ID`,
  instrument_type: t('tradingConfig.instrument_type'),
  side: t('tradingConfig.side'),
  offset: t('tradingConfig.offset'),
  hedge_flag: t('tradingConfig.hedge'),
  price_type: t('tradingConfig.price_type'),
  volume: t('tradingConfig.volume'),
  exchange_id: `${t('globalSettingConfig.exchange_id')}ID`,
  limit_price: t('tradingConfig.limit_price'),
};
