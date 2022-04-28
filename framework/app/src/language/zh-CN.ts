export default {
  keyword_input: '关键字',
  empty_text: '暂无数据',

  baseConfig: {
    main_panel: '主面板',
    control_center: '控制中心',
  },

  tradingConfig: {
    un_known: '未知',

    running: '运行中',
    stopping: '停止中',
    stopped: '已停止',
    launching: '启动中',
    error: '错误',
    waiting_restart: '待重启',

    pending: '等待中',
    Idle: '无数据',
    dis_connected: '已断开',
    connected: '已连接',
    logged_in: '已登录',
    login_failed: '登录失败',
    ready: '就绪',

    system: '系统服务',
    daemon: '后台服务',
    md: '行情源',
    td: '交易账户 ',
    strategy: '策略',

    open: '开',
    close: '平',
    close_today: '平今',
    close_yesterday: '平昨',

    buy: '买',
    sell: '卖',
    lock: '锁仓',
    unlock: '解锁',
    exec: '行权',
    drop: '放弃行权',
    purchase: '申购',
    redemption: '赎回',
    split: '拆分',
    merge: '合并',
    margin_trade: '融资买入',
    short_sell: '融券卖出',
    repay_margin: '卖券还款',
    repay_short: '买券还券',
    cash_repay_margin: '现金还款',
    stock_repay_short: '现券还券',
    surplus_stock_transfer: '余券划转',
    guarantee_stock_transfer: '担保品转入',
    guarantee_stock_redeem: '担保品转出',

    submitted: '已提交',
    cancelled: '已撤单',
    filled: '已成交',
    partial_filled_not_active: '部分撤单',
    partial_filled_active: '正在交易',
    lost: '丢失',

    long: '多',
    short: '空',

    Limit: '[Limit] 限价',
    Market: '[Any] 市价',
    FakBest5: '[FakBest5] 上海深圳最优五档即时成交剩余撤销, 不需要报价',
    Forward_best: '[ForwardBest] 深圳本方方最优价格申报',
    Reverse_best:
      '[ReverseBest] 上海最优五档即时成交剩余转限价, 深圳对手方最优价格申报, 不需要报价',
    Fak: '[Fak] 深圳即时成交剩余撤销',
    Fok: '[Fok] 深圳市价全额成交或者撤销',

    speculation: '投机',
    hedge: '套保',
    arbitrage: '套利',
    covered: '备兑',

    any: '任意',
    min: '最小',
    all: '全部',

    by_amount: '按金额',
    by_volume: '按手数',

    stock: '普通股票',
    future: '期货',
    bond: '债券',
    stock_option: '股票期权',
    fund: '基金',
    tech_stock: '科技板股票',
    index: '指数',
    repo: '回购',
    crypto: '数字货币',
    crypto_future: '数字货币合约',
    kungfu_simu: '功夫模拟',

    SSE: '上交所',
    SZE: '深交所',
    BSE: '北交所',
    SHFE: '上期所',
    DCE: '大商所',
    CZCE: '郑商所',
    CFFEX: '中金所',
    INE: '能源中心',

    SP: '郑商所 跨期 SP',
    SPC: '郑商所 跨品种 SPC',
    SPD: '大商所 跨期 SPD',
    IPS: '大商所 跨品种 IPS',

    master: '主控进程',
    ledger: '计算服务',
    cached: '存储服务',
    archive: '归档服务',

    place_order: '下单',
    apart_order: '拆单',
    account: '账户',
    instrument: '标的',
    volume: '下单量',
    price: '价格',
    protect_price: '保护价格',
    price_type: '方式',
    side: '买卖',
    offset: '开平',
  },

  orderConfig: {
    update_time: '更新时间',
    instrument_id: '代码',
    limit_price: '委托价',
    order_status: '订单状态',
    latency_system: '系统延迟μs',
    latency_network: '网络延迟μs',
    dest_uname: '下单源',
    source_uname: '目标账户',
    completed: '已完成',
    clinch: '已成交',
    all: '全部',

    mean: '委托均价',
    max: '最高委托价',
    min: '最低委托价',
    volume: '成交量',

    checkbox_text: '未完成委托',
    cancel_all: '全部撤单',
    date_check: '请选择日期',
  },

  tdConfig: {
    account_name: '账户(组)',
    account_ps: '备注',
    state_status: '状态',
    process_status: '进程',
    unrealized_pnl: '浮动盈亏',
    marked_value: '市值',
    margin: '保证金',
    avail_money: '可用资金',
    actions: '操作',

    add_td: '添加',
    add_group_placeholder: '添加分组',
  },

  mdConfig: {
    counter_name: '柜台',
    state_status: '状态',
    process_status: '进程',
    actions: '操作',

    add_md: '添加',
  },

  strategyConfig: {
    strategy_id: '策略ID',
    strategy_file: '策略文件',
    process_status: '进程',
    unrealized_pnl: '浮动盈亏',
    marked_value: '市值',
    actions: '操作',

    add_strategy: '添加',
  },

  tradingTaskConfig: {
    task_id: '任务ID',
    process_status: '进程',
    args: '参数',
    actions: '操作',

    add_task: '添加',
  },

  posGlobalConfig: {
    instrument_id: '代码',
    yesterday_volume: '昨',
    today_volume: '今',
    sum_volume: '总',
    avg_open_price: '开仓均价',
    last_price: '最新价',
    unrealized_pnl: '浮动盈亏',
  },

  marketDataConfig: {
    instrument_id: '标的',
    open_price: '开盘价',
    high_price: '最高价',
    low_price: '最低价',
    last_price: '最新价',
    volume: '成交量',
    actions: '操作',

    subscribe_btn: '订阅',
    add_market: '添加自选',
  },

  tradeConfig: {
    instrument: '标的',
    trade_time_resolved: '成交时间',
    kf_time_resolved: '系统时间',
    instrument_id: '代码',
    price: '成交价',
    volume: '成交量',
    latency_trade: '成交延迟(μs)',

    mean_price: '成交均价',
    min_price: '最低成交价',
    max_price: '最高成交价',
  },

  futureArbitrageConfig: {
    account_name: '账户',
    future_arbitrage_code: '套利代码',
    instrument_bug_A: '买入标的 A',
    instrument_sell_A: '卖出标的 A',
    instrument_bug_B: '买入标的 B',
    instrument_sell_B: '卖出标的 B',
    side: '买卖',
    offset: '开平',
    hedge_flag: '套保',
    price_type: '方式',
    limit_price: '价差 A-B',
    volume: '下单量',
  },

  MakeOrderDashboard: {
    name: '下单面板',
  },
  FutureArbitrage: {
    name: '套利指令',
  },
  OrderBook: {
    name: '深度行情',
  },
  MarketData: {
    name: '行情订阅',
  },
  TradingTask: {
    name: '交易任务',
  },
  Strategy: {
    name: '策略进程',
  },
  Md: {
    name: '行情源 ',
  },
  Td: {
    name: '交易账户',
  },
  Trade: {
    name: '成交记录',
  },
  Order: {
    name: '委托记录',
  },
  PosGlobal: {
    name: '持仓汇总',
  },
  Pos: {
    name: '持仓',
  },
};
