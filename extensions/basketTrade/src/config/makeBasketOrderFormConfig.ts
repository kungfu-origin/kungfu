import {
  SideEnum,
  PriceTypeEnum,
  PriceLevelEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getMakeBasketOrderConfigSettings =
  (): KungfuApi.KfConfigItem[] => [
    {
      type: 'basket',
      key: 'basket',
      name: t('tradingConfig.account'),
      required: true,
    },
    {
      type: 'tds',
      key: 'accountIds',
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
      tip: t('BasketTrade.price_level_tip'),
      default: PriceLevelEnum.Latest,
    },
    {
      type: 'float',
      key: 'priceOffset',
      name: t('tradingConfig.price_offset'),
      tip: t('BasketTrade.price_offset_tip', {
        prefix: t('BasketTrade.price_level_tip') + ', ',
      }),
      min: 0,
      default: 0,
    },
    {
      type: 'int',
      key: 'basketVolume',
      name: t('BasketTrade.basket_volume'),
      tip: t('BasketTrade.basket_volume_tip'),
      min: 1,
      default: 1,
    },
  ];

export const getChaseBasketOrderConfigSettings =
  (): KungfuApi.KfConfigItem[] => [
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
      tip: t('BasketTrade.price_offset_tip'),
      min: 0,
      default: 0,
    },
  ];

export const getReplenishBasketOrderConfigSettings =
  (): KungfuApi.KfConfigItem[] => [
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
      tip: t('BasketTrade.price_offset_tip'),
      min: 0,
      default: 0,
    },
  ];
