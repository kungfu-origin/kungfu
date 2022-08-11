import {
  KfCategoryTypes,
  PriceTypeEnum,
  SideEnum,
} from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
const { t } = VueI18n.global;

export const getConfigSettings = (
  category?: KfCategoryTypes,
): KungfuApi.KfConfigItem[] =>
  [
    category === 'td'
      ? null
      : {
          key: 'account_id',
          name: t('blockTradeConfig.account_name'),
          type: 'td',
          required: true,
        },
    {
      key: 'instrument',
      name: t('blocktradeConfig.instrument_id'),
      type: 'instrument',
      required: true,
    },
    {
      key: 'side',
      name: t('blockTradeConfig.side'),
      type: 'side',
      default: SideEnum.Buy,
      required: true,
    },
    {
      key: 'price_type',
      name: t('blockTradeConfig.price_type'),
      type: 'priceType',
      default: PriceTypeEnum.Limit,
      required: true,
    },
    {
      key: 'limit_price',
      name: t('blockTradeConfig.limit_price'),
      type: 'float',
      required: true,
    },
    {
      key: 'volume',
      name: t('blockTradeConfig.volume'),
      type: 'int',
      required: true,
    },
    {
      key: 'opponent_seat',
      name: t('blockTradeConfig.opponent_seat'),
      type: 'str',
      required: true,
    },
    {
      key: 'opponent_account',
      name: t('blockTradeConfig.opponent_account'),
      type: 'str',
      required: true,
    },
    {
      key: 'match_number',
      name: t('blockTradeConfig.match_number'),
      type: 'str',
      required: true,
    },
    {
      key: 'linkman',
      name: t('blockTradeConfig.linkman'),
      type: 'str',
      required: false,
    },
    {
      key: 'contact_way',
      name: t('blockTradeConfig.contact_way'),
      type: 'str',
      required: false,
    },
    {
      key: 'underweight_type',
      name: t('blockTradeConfig.underweight_type'),
      type: 'underweightType',
      required: true,
    },
  ].filter((item) => !!item) as KungfuApi.KfConfigItem[];

export const getBlockTradeOrderTrans = (): Record<string, string> => {
  return getConfigSettings('td').reduce((pre, cur) => {
    pre[cur.key] = cur.name;
    return pre;
  }, {});
};
