import VueI18n from '@kungfu-trader/kungfu-app/src/language';
const { t } = VueI18n.global;

export const getColumns = (smallMode = false): AntTableColumns => [
  {
    title: t('marketDataConfig.instrument_id'),
    dataIndex: 'instrumentId',
    align: 'left',
    width: 80,
  },
  ...(smallMode
    ? []
    : [
        {
          title: t('marketDataConfig.open_price'),
          dataIndex: 'open_price',
          align: 'right',
          width: 80,
        },
        {
          title: t('marketDataConfig.low_price'),
          dataIndex: 'low_price',
          align: 'right',
          width: 80,
        },
        {
          title: t('marketDataConfig.high_price'),
          dataIndex: 'high_price',
          align: 'right',
          width: 80,
        },
      ]),
  {
    title: t('marketDataConfig.last_price'),
    dataIndex: 'lastPrice',
    align: 'right',
    width: 80,
  },
  ...(smallMode
    ? []
    : [
        {
          title: t('marketDataConfig.volume'),
          dataIndex: 'volume',
          align: 'right',
          width: 60,
        },
      ]),
  {
    title: t('marketDataConfig.actions'),
    dataIndex: 'actions',
    align: 'right',
    width: 40,
    fixed: 'right',
  },
];
