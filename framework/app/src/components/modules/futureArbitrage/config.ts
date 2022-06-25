import {
  HedgeFlagEnum,
  KfCategoryTypes,
  OffsetEnum,
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getConfigSettings = (
  category?: KfCategoryTypes,
  side?: SideEnum | undefined,
): KungfuApi.KfConfigItem[] =>
  [
    category === 'td'
      ? null
      : {
          key: 'account_id',
          name: t('futureArbitrageConfig.account_name'),
          type: 'td',
          required: true,
        },
    {
      key: 'future_arbitrage_code',
      name: t('futureArbitrageConfig.future_arbitrage_code'),
      type: 'futureArbitrageCode',
      required: true,
    },
    {
      key: 'instrument_A',
      name:
        side === SideEnum.Buy
          ? t('futureArbitrageConfig.instrument_buy_A')
          : t('futureArbitrageConfig.instrument_sell_A'),
      type: 'instrument',
      required: true,
    },
    {
      key: 'instrument_B',
      name:
        side === SideEnum.Buy
          ? t('futureArbitrageConfig.instrument_sell_B')
          : t('futureArbitrageConfig.instrument_buy_B'),
      type: 'instrument',
      required: true,
    },
    {
      key: 'side',
      name: t('futureArbitrageConfig.side'),
      type: 'side',
      default: SideEnum.Buy,
      required: true,
    },
    {
      key: 'offset',
      name: t('futureArbitrageConfig.offset'),
      type: 'radio',
      options: [
        {
          label: t('tradingConfig.open'),
          value: OffsetEnum.Open.toString(),
        },
        {
          label: t('tradingConfig.close'),
          value: OffsetEnum.Close.toString(),
        },
      ],
      default: OffsetEnum.Open.toString(),
      required: true,
    },
    {
      key: 'hedge_flag',
      name: t('futureArbitrageConfig.hedge_flag'),
      type: 'hedgeFlag',
      default: HedgeFlagEnum.Speculation,
      required: true,
    },
    {
      key: 'price_type',
      name: t('futureArbitrageConfig.price_type'),
      type: 'priceType',
      default: PriceTypeEnum.Limit,
      required: true,
    },
    {
      key: 'limit_price',
      name: t('futureArbitrageConfig.limit_price'),
      type: 'float',
      required: true,
    },
    {
      key: 'volume',
      name: t('futureArbitrageConfig.volume'),
      type: 'int',
      required: true,
    },
  ].filter((item) => !!item) as KungfuApi.KfConfigItem[];
