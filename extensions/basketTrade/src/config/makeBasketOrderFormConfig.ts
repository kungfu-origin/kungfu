import {
  SideEnum,
  OffsetEnum,
  PriceTypeEnum,
  PriceLevelEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getMakeBasketOrderConfigSettings =
  (): KungfuApi.KfConfigItem[] => [
    {
      type: 'td',
      key: 'accountId',
      name: t('tradingConfig.account'),
      required: true,
    },
    {
      type: 'side',
      key: 'side',
      name: t('tradingConfig.side'),
      default: SideEnum.Buy,
      required: true,
    },
    {
      type: 'offset',
      key: 'offset',
      name: t('tradingConfig.offset'),
      default: OffsetEnum.Open,
      required: true,
    },
    {
      type: 'priceType',
      key: 'priceType',
      name: t('tradingConfig.price_type'),
      default: PriceTypeEnum.Limit,
      required: true,
    },
    {
      type: 'priceLevel',
      key: 'priceLevel',
      name: t('tradingConfig.price_level'),
      default: PriceLevelEnum.Latest,
      required: true,
    },
    {
      type: 'float',
      key: 'priceOffset',
      name: t('tradingConfig.price_offset'),
      min: 0,
      default: 0,
      required: true,
    },
  ];

export const getChaseBasketOrderConfigSettings =
  (): KungfuApi.KfConfigItem[] => [
    {
      type: 'priceLevel',
      key: 'priceLevel',
      name: t('tradingConfig.price_level'),
      tip: t('BasketTrade.chase_order_tip'),
      default: PriceLevelEnum.Latest,
      required: true,
    },
    {
      type: 'float',
      key: 'priceOffset',
      name: t('tradingConfig.price_offset'),
      tip: t('BasketTrade.price_offset_tip'),
      min: 0,
      default: 0,
      required: true,
    },
  ];

export const getReplenishBasketOrderConfigSettings =
  (): KungfuApi.KfConfigItem[] => [
    {
      type: 'priceLevel',
      key: 'priceLevel',
      name: t('tradingConfig.price_level'),
      tip: t('BasketTrade.replenish_order_tip'),
      default: PriceLevelEnum.Latest,
      required: true,
    },
    {
      type: 'float',
      key: 'priceOffset',
      name: t('tradingConfig.price_offset'),
      tip: t('BasketTrade.price_offset_tip'),
      min: 0,
      default: 0,
      required: true,
    },
  ];
