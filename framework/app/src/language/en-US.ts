export default {
  baseConfig: {
    mainPanel: 'Main',
    controlCenter: 'Control',
  },

  tradingConfig: {
    placeOrder: 'Place Order',
    apartOrder: 'Apart Order',
    account: 'Account',
    instrument: 'Instrument',
    volume: 'Volume',
    price: 'Price',
    protectPrice: 'Protect Price',
    priceType: 'Order Type',
    sell: 'Sell',
    buy: 'Buy',
    side: 'Side',
    offset: 'Offset',
    hedgeFlag: 'Hedge Flag',
    open: 'Open',
    close: 'Close',
  },

  orderConfig: {
    updateTime: 'update_time',
    instrumentId: 'instruemnt_id',
    limitPrice: 'limit_price',
    orderStatus: 'order_status',
    latencySystem: 'latency_system(μs)',
    latencyNetwork: 'latency_network(μs)',
    destUname: 'dest_uname',
    sourceUname: 'source_uname',
    completed: 'completed',
    clinch: 'clinch',
    all: 'all',

    mean: 'mean_price',
    max: 'max_price',
    min: 'min_price',
    volume: 'volume',
  },

  tdConfig: {},

  MakeOrderDashboard: {
    name: 'Orderpanel',
  },
  FutureArbitrage: {
    name: 'Arbitrage',
  },
  OrderBook: {
    name: 'Orderbook',
  },
  MarketData: {
    name: 'Marketdata',
  },
  TradingTask: {
    name: 'Tradingtask',
  },
  Strategy: {
    name: 'Strategy',
  },
  Md: {
    name: 'md ',
  },
  Td: {
    name: 'td',
  },
  Trade: {
    name: 'traderecord',
  },
  Order: {
    name: 'orderrecord',
  },
  PosGlobal: {
    name: 'positionsummary',
  },
  Pos: {
    name: 'positon',
  },
};
