export default {
  keyword_input: 'keyword',
  empty_text: 'no data',
  success: 'success',
  error: 'error',
  add: 'Add',
  close: 'Close',
  delete: 'delete',
  rename: 'rename',
  set: 'Set',
  clean: 'clean',
  Help: 'Help',
  run: 'Run',
  quit: 'Quit',
  operation_success: 'operation success',
  operation_failed: 'operation failed',
  location_error: 'current location error',
  watcher_error: 'Watcher error',
  instrument_error: 'instrument error',
  prompt: 'prompt',
  warning: 'warning',
  confirm: 'confirm',
  cancel: 'cancel',
  save_file: 'save file',
  detail: 'detail',
  ok: 'ok',

  edit: 'Edit',
  copy: 'Copy',
  paste: 'Paste',
  select_all: 'Select all',
  undo: 'Undo',

  open_resources_directory: 'Open Kungfu Resources directory (KF_HOME)',
  open_install_directory: 'Open Kungfu installation directory',
  open_basic_configuration: 'Open Kungfu basic configuration DB',
  browsing_log: 'Browsing log files',

  clear_journal: 'Clear journal',
  clear_DB: 'Clear DB',
  reset_main_panel: 'Reset main panel',
  export_all_transaction_data: 'E`xport all transaction data',

  website: 'Website',
  user_manual: 'User manual',
  API_documentation: 'Strategy API documentation',
  Kungfu_forum: 'Kungfu forum',

  KungFu: 'Kungfu',
  about_kungfu: 'About Kungfu',

  file: 'file',
  folder: 'folder',

  no_focus: 'The current page is not in focus',
  clear: 'Clearing {content} completed, please restart the application',
  open_window: 'Opening window',
  open_code_editor: 'Opening code editor',
  add_board: 'Add Board',
  select_board: 'Please select the board to add',
  add_board_error: 'Add board target error',

  delete_category:
    'Delete all data in {category}, if the {categoryName} process is running, the process will also be stopped, confirm deletion',
  add_config_modal:
    '{category} ID unique in the system. {changeTypeName} cannot be modified after it succeeds.',
  update_config_modal: 'Confirm {key} configurations',

  MakeOrderDashboard: 'Order Dashboard',
  FutureArbitrage: 'Future Arbitrage',
  OrderBook: 'Order Book',
  MarketData: 'Market Data',
  TradingTask: 'Trading Task',
  Strategy: 'Strategy',
  Md: 'Md',
  Td: 'Td',
  Trade: 'Trade',
  Order: 'Order',
  PosGlobal: 'Positon Global',
  Pos: 'Positon',

  baseConfig: {
    main_panel: 'HOME',
    control_center: 'Control',
  },

  tradingConfig: {
    un_known: 'Unknown',
    default: 'Default',
    order_task: 'Trade',

    running: 'Running',
    stopping: 'Stopping',
    stopped: 'Stopped',
    launching: 'Launching',
    error: 'Errored',
    waiting_restart: 'Stopped',

    pending: 'Pending',
    Idle: 'Idle',
    dis_connected: 'Disconnected',
    connected: 'Connected',
    logged_in: 'LoggedIn',
    login_failed: 'LoginFailed',
    ready: 'Ready',

    system: 'System',
    daemon: 'Daemon',
    md: 'Md',
    td: 'Td',
    strategy: 'Strategy',

    open: 'Open',
    close: 'Close',
    close_today: 'CloseToday',
    close_yesterday: 'CloseYest',

    buy: 'Buy',
    sell: 'Sell',
    lock: 'Lock',
    unlock: 'Unlock',
    exec: 'Exec',
    drop: 'Drop',
    purchase: 'Purchase',
    redemption: 'Redemption',
    split: 'Split',
    merge: 'Merge',
    margin_trade: 'MarginTrade',
    short_sell: 'ShortSell',
    repay_margin: 'RepayMargin',
    repay_short: 'RepayShort',
    cash_repay_margin: 'CashRepayMargin',
    stock_repay_short: 'StockRepayShort',
    surplus_stock_transfer: 'SurplusStockTransfer',
    guarantee_stock_transfer: 'GuaranteeStockTransferIn',
    guarantee_stock_redeem: 'GuaranteeStockTransferOut',

    submitted: 'Submitted',
    cancelled: 'Cancelled',
    filled: 'Filled',
    partial_filled_not_active: 'PartialFilledNotActive',
    partial_filled_active: 'PartialFilledActive',
    lost: 'Lost',

    long: 'Long',
    short: 'Short',

    Limit: 'Limit',
    Market: 'Market',
    FakBest5: 'FakBest5',
    Forward_best: 'ForwardBest',
    Reverse_best: 'ReverseBest',
    Fak: 'Fak',
    Fok: 'Fok',

    speculation: 'Speculation',
    hedge: 'Hedge',
    arbitrage: 'Arbitrage',
    covered: 'Covered',

    any: 'Any',
    min: 'Min',
    all: 'All',

    by_amount: 'ByAmount',
    by_volume: 'ByVolume',

    IOC: 'IOC',
    GFD: 'GFD',
    GTC: 'GTC',

    stock: 'Stock',
    future: 'Future',
    bond: 'Bond',
    stock_option: '股票期权',
    fund: 'Fund',
    tech_stock: 'TeckStock',
    index: 'Index',
    repo: 'Repo',
    crypto: 'Crypto',
    crypto_future: 'CryptoFuture',
    kungfu_simu: 'KungfuSimu',

    SSE: 'SSE',
    SZE: 'SZE',
    BSE: 'BSE',
    SHFE: 'SHFE',
    DCE: 'DCE',
    CZCE: 'CZCE',
    CFFEX: 'CFFEX',
    INE: 'INE',

    SP: 'SP',
    SPC: 'SPC',
    SPD: 'SPD',
    IPS: 'IPS',

    master: 'master',
    ledger: 'ledger',
    cached: 'cached',
    archive: 'archive',

    place_order: 'Place Order',
    apart_order: 'Apart Order',
    reset_order: 'Reset',
    account: 'AccountId',
    instrument: 'Instrument',
    instrument_type: 'Instrument Type',
    volume: 'Volume',
    price: 'Price',
    protect_price: 'Protect Price',
    price_type: 'Price Type',
    side: 'Side',
    offset: 'Offset',
    limit_price: 'LimitPrice',

    make_order_number: 'Order Number',
    no_empty: 'Order quantity cannot be empty',
    total_order_amount: 'Total Order Amount',
    every_volume: 'Every Volume',

    fat_finger_buy_modal:
      'The buying price exceeded the warning line, the current price is {price}, line for {warningLine}, fat finger is {fatFinger}%',
    fat_finger_sell_modal:
      'The selling price exceeded the warning line, the current price is {price}, line for {warningLine}, fat finger is {fatFinger}%',
    start_process: 'please start {process} first',
    place_confirm: 'Place Order Confirm',
    close_all: 'Close All?',
    Continue: 'Continue',
  },

  orderConfig: {
    update_time: 'update_time',
    instrument_id: 'instrument_id',
    limit_price: 'limit_price',
    order_status: 'order_status',
    latency_system: 'latency_system(μs)',
    latency_network: 'latency_network(μs)',
    dest_uname: 'dest_uname',
    source_uname: 'source_uname',
    completed: 'completed',
    clinch: 'clinch',
    all: 'all',

    mean: 'mean',
    max: 'max',
    min: 'min',
    volume: 'volume',

    checkbox_text: 'Outstanding delegate',
    cancel_all: 'All cancellations',
    date_picker: 'Select a date',
    confirm_cancel_all: 'Confirm cancellation of all orders',
    entrust: 'Entrust',
    start: 'Please Start',
    trade_process: 'Trade Process',

    entrust_statistical: 'Entrust Statistical',
    statistical_desc: 'Real-time (latest 100 pieces of data)',
    entrust_statistical_number: 'Entrust Statistical Number',
    ntrust_statistical_price: 'Entrust Statistical Price',
    average_withdrawal_ratio:
      'Average withdrawal ratio (only partial withdrawal and total withdrawal of statistics Department)',
    average_system_latency: 'Average System Latency(μs)',
    min_system_latency: 'Min System Latency(μs)',
    max_system_latency: 'Max System Latency(μs)',
    average_network_latency: 'Average Network Latency(μs)',
    min_network_latency: 'Min Network Latency(μs)',
    max_network_latency: 'Max Network Latency(μs)',
  },

  tdConfig: {
    td_group: 'Td Group',
    td_name: 'Td Group Name',
    account_name: 'account',
    account_ps: 'note',
    state_status: 'status',
    process_status: 'process',
    unrealized_pnl: 'unrealized_pnl',
    marked_value: 'marked_value',
    margin: 'margin',
    avail_money: 'avail_money',
    actions: 'actions',

    add_td: 'Add',
    add_group_placeholder: 'Add group',
    set_td_group: 'Account Group Settings',
    account_group: 'Account Group',
    td_not_found: '{td} Td not found',
    sourse_not_found:
      'The configuration item does not exist, please check {value} package.json',
    need_only_group: 'Ensure that the account group name is unique',
    delete_amount_group: 'Deleting account Group {group}',
    confirm_delete_group:
      'The account process under account group change will not be affected, confirm the deletion',
  },

  mdConfig: {
    counter_name: 'counter_name',
    state_status: 'status',
    process_status: 'process',
    actions: 'actions',
    select_counter_api: 'Select a counter',
    select_trade_task: 'Select trade task',
    select_plugin_type: 'Select plugin type',

    add_md: 'Add',
    counter_plugin_inexistence: 'Counter plugin inexistence',
  },

  strategyConfig: {
    strategy: 'strategy',
    strategy_id: 'strategy_id',
    strategy_file: 'strategy_file',
    strategy_path: 'strategy_path',
    process_status: 'process',
    unrealized_pnl: 'unrealized_pnl',
    marked_value: 'marked_value',
    actions: 'actions',
    strategy_tip: 'Ensure that the strategy_id is unique',

    add_strategy: 'Add',
  },

  tradingTaskConfig: {
    task_id: 'task_id',
    process_status: 'process',
    args: 'arguments',
    actions: 'actions',

    add_task: 'Add',
    illegal_process_id: 'Not a legitimate trade task process_id',
    key_inexistence: 'The trade task plugin key does not exist',
    plugin_inexistence: 'The trade task plugin does not exist',
    configuration_inexistence:
      'The configuration item does not exist,please check',
    delete_task: 'Deleting a trade Task',
    delete_task_content:
      'all data, if the transaction task is running, will also stop the process and confirm deletion',
  },

  posGlobalConfig: {
    instrument_id: 'instrument_id',
    account_id_resolved: 'account_id_resolved',
    yesterday_volume: 'yesterday_volume',
    today_volume: 'today_volume',
    sum_volume: 'sum_volume',
    avg_open_price: 'avg_open_price',
    last_price: 'last_price',
    unrealized_pnl: 'unrealized_pnl',
  },

  marketDataConfig: {
    instrument_id: 'instrument_id',
    open_price: 'open_price',
    high_price: 'high_price',
    low_price: 'low_price',
    last_price: 'last_price',
    volume: 'volume',
    actions: 'actions',

    subscribe_btn: 'Subscribe',
    add_market: 'Add',
  },

  tradeConfig: {
    instrument: 'instrument',
    trade_time_resolved: 'trade_time',
    kf_time_resolved: 'kf_time',
    instrument_id: 'instrument_id',
    price: 'price',
    volume: 'volume',
    latency_trade: 'latency_trade(μs)',

    mean_price: 'Mean Price',
    min_price: 'Min Price',
    max_price: 'Max Price',

    statistical: 'Transaction Statistical',
    statistical_count: 'Statistics of transaction quantity',
    statistical_price: 'Transaction price statistics',
    statistical_desc: 'Real-time (latest 100 pieces of data)',
    average_trade_latency: 'Average transaction delay(μs)',
    max_trade_latency: 'Max transaction delay(μs)',
    min_trade_latency: 'Min transaction delay(μs)',
  },

  futureArbitrageConfig: {
    account_name: 'account_name',
    future_arbitrage_code: 'future_arbitrage_code',
    instrument_bug_A: 'instrument_bug_A',
    instrument_sell_A: 'instrument_sell_A',
    instrument_bug_B: 'instrument_bug_B',
    instrument_sell_B: 'instrument_sell_B',
    side: 'side',
    offset: 'offset',
    hedge_flag: 'hedge_flag',
    price_type: 'price_type',
    limit_price: 'limit_price A-B',
    volume: 'volume',

    place_order: 'Place Order',
    reset_order: 'Reset',
    only_corresponding: 'Can Only corresponding',
  },

  globalSettingConfig: {
    global_setting_title: 'Global Setting',
    system: 'System',
    log_level: 'Log Level',
    for_all_log: 'For all Log',

    language: 'Language',
    select_language: 'Select Language',

    porformance: 'Performance',
    open_rocket_model: 'Open Rocket Model',
    rocket_model_desc: 'Use CPU 100%, restart is required',
    pass_quote: 'Close Quote Listener',
    pass_quote_desc: 'Close Quote Listener',

    strategy: 'Strategy',
    use_local_python: 'Use Local Python',
    local_python_desc:
      'Pip3 install *.whl in {whl_dir_path}, local python version require {py_version}',
    python_path: 'Select Local Python Path',
    python_path_desc:
      'local python path is required to be selected, and kungfu*.whl should be installed in this path',

    trade: 'Trade',
    sound: 'Traded Sound',
    use_sound: 'Enable the system traded prompt sound',
    fat_finger_threshold: 'Fat Finger Threshold',
    set_fat_finger:
      'Set the threshold for triggering an fat finger(percentage)',
    close_threshold: 'Close Threshold',
    set_close_threshold:
      'Set the threshold for triggering an close(percentage)',

    code_editor: 'Editor',
    tab_space_type: 'Indentation Category',
    set_tab_space: 'Kungfu Editor Indentation Category',
    tab_space_size: 'Indentation Size',
    set_tab_space_size: 'Kungfu Editor Indentation Size (space)',

    comission: 'Comission',
    varieties: 'Varieties',
    add_comission: 'Add',
    exchange_id: 'Exchange ID',
    open: 'Open',
    close_today: 'Close Today',
    close_yesterday: 'Close Yesterday',
    min: 'Min',

    timing_rev_top: 'Timing Rev. Top',
    use_timing_rev_top: 'Enable Timing Rev. Top',
    timing_task_list: 'Timing Task List',
    add_timing: 'Add Timing',
    target_process: 'Target Process',
    manner: 'Manner',
    daily_time: 'Daily Time',

    master: 'master',
    start: 'start',
    stop: 'stop',
    restart: 'restart',
  },

  风控: 'Risk Setting',
  风控描述:
    'Enable risk setting to automatically close positions when positions exceed thresholds',
  账户: 'Account',
  柜台: 'Exchange',
  单比最大量: 'Maximum single ratio',
  每日最大成交量: 'Maximum daily turnover',
  防止自成交: 'Whether automatic success detection is required',
  最大回撤率: 'Maximum withdrawal rate',
  标的白名单: 'white list',
  白名单设置警告: 'Please set the whitelist for this account first',

  validate: {
    no_special_characters: 'Cannot contain special characters',
    no_underscore: 'Cannot contain underscores',
    no_zero_number: 'Cannot contain zero',
    no_negative_number: 'Cannot contain negative',
    value_existing: '{value} has been in existence',
    mandatory: 'mandatory',
  },

  editor: {
    set_strategy_entrance: 'Set Strategy Entry',
    current_strategy: 'Current strategy',
    new_file: 'New File',
    new_folder: 'New Folder',
    set_strategy_success: 'Strategy {file} file path changed successfully!',
    entry_file: 'Entry File',
    creaate_success: '{file} created successfully!',

    name_repeat:
      'This location already exists file or folder {name}, please select a different name!',
    empty_input: 'File or folder name must be provided!',
    illegal_character: 'The name cannot contain \\/:*?" <>|',
    delate_confirm: 'Are you sure to delete {value}?',
    cannot_delate_entry: 'Cannot delete the entry',
  },

  logview: {
    scroll_to_bottom: 'Scroll to bottom',
  },

  master_interrupt: 'master interrupted',
  master_desc:
    'The master process is responsible for inter-policy process communication and resource configuration, please restart the application',

  ledger_interrupt: 'ledger interrupted',
  ledger_desc:
    'Accounting services are responsible for position and capital calculations, please restart the application',

  cached_interrupt: 'cached interrupted',
  cached_desc:
    'The storage service is responsible for data landing. The storage service disconnection does not affect transactions, please restart the application after the transaction',

  state_interrupt_msg: '{state} has been disconnected',
  state_interrupt_desc:
    '{state} has been disconnected, transaction may be interrupted. Please check',

  kungfu: 'Kungfu',

  system_prompt: 'System Prompt',
  archive_completion: 'Archive Completion',
  archive: 'Kungfu archive...',
  environment_complete: 'Kungfu environment ready to complete ✓',
  environment_preparation: 'Kungfu environment in preparation...',
  ready: 'Kungfu ready ✓',
  wait_ready: 'Wait for Kungfu...',
  saving_completed: 'Data saving completed ✓',
  save_data: 'Save Data...',
  end_all_transactions: 'End all transactions ✓',
  closing: 'End the transaction process, Please do not close...',

  quit_confirm:
    'Exiting the application ends all transactions, Confirm to exit?',
  restart_process:
    'Kungfu graphics process is interrupted. The interruption will not affect transactions. Do you want to restart the graphics process?',

  未就绪: '{processId} not ready, please try again later',
  系统外: 'Out-of-system',
  手动: 'manual',
  任务: 'task',
};
