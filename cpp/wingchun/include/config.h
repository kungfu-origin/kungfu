//
// Created by qlu on 2019/2/15.
//

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

namespace kungfu
{
// app id
#define KF_APP_ID "TAURUS_KUNGFU_2.0"

// uid generator configuration
#define UID_EPOCH_SECONDS 1463673600 // "2016-05-20"
#define UID_WORKER_DB_FILE_FORMAT "{}/global/uid_worker.db"

#define UID_WORKER_DB_FILE fmt::format(UID_WORKER_DB_FILE_FORMAT, get_base_dir())

// future instrument info configuration
#define FUTURE_INSTRUMENT_DB_FILE_FORMAT "{}/global/future_instrument.db"

#define FUTURE_INSTRUMENT_DB_FILE fmt::format(FUTURE_INSTRUMENT_DB_FILE_FORMAT, get_base_dir())

// calendar configuration
#define CALENDAR_HOLIDAY_DB_FILE_FORMAT "{}/global/holidays.db"
#define CALENDAR_REP_URL "ipc://{}/socket/service.sock"
#define CALENDAR_PUB_URL "ipc://{}/socket/notice.sock"

// gateway configuration

#define MD_GATEWAY_NAME_FORMAT "md_{}"
#define TD_GATEWAY_NAME_FORMAT "td_{}_{}"

#define MD_GATEWAY_NAME(source) fmt::format(MD_GATEWAY_NAME_FORMAT, source)
#define TD_GATEWAY_NAME(source, account_id) fmt::format(TD_GATEWAY_NAME_FORMAT, source, account_id)

#define GATEWAY_STATE_DB_FILE_FORMAT "{}/gateway/{}/state.db"
#define SUBSCRIPTION_DB_FILE_FORMAT "{}/gateway/{}/subscription.db"
#define ORDER_MAPPER_DB_FILE_FORMAT "{}/gateway/{}/order_mapper.db"

#define GATEWAY_PUB_URL_FORMAT "ipc://{}/gateway/{}/pub.ipc"
#define GATEWAY_REP_URL_FORMAT "ipc://{}/gateway/{}/rep.ipc"

#define GATEWAY_FOLDER(name) fmt::format("{}/gateway/{}", get_base_dir(), name)
#define GATEWAY_STATE_DB_FILE(name) fmt::format(GATEWAY_STATE_DB_FILE_FORMAT, get_base_dir(), name)
#define SUBSCRIPTION_DB_FILE(name) fmt::format(SUBSCRIPTION_DB_FILE_FORMAT, get_base_dir(), name)
#define ORDER_MAPPER_DB_FILE(name) fmt::format(ORDER_MAPPER_DB_FILE_FORMAT, get_base_dir(), name)
#define GATEWAY_PUB_URL(name) fmt::format(GATEWAY_PUB_URL_FORMAT, get_base_dir(), name)
#define GATEWAY_REP_URL(name) fmt::format(GATEWAY_REP_URL_FORMAT, get_base_dir(), name)

// journal related configuration
#define STRATEGY_JOURNAL_FOLDER_FORMAT "{}/journal/strategy/"
#define MD_JOURNAL_FOLDER_FORMAT "{}/journal/md/{}"
#define TD_JOURNAL_FOLDER_FORMAT "{}/journal/td/{}/{}"

#define MD_JOURNAL_NAME(source_id) MD_GATEWAY_NAME(source_id)
#define TD_JOURNAL_NAME(source_id, account_id) TD_GATEWAY_NAME(source_id, account_id)

#define STRATEGY_JOURNAL_FOLDER fmt::format(STRATEGY_JOURNAL_FOLDER_FORMAT, get_base_dir())
#define MD_JOURNAL_FOLDER(source) fmt::format(MD_JOURNAL_FOLDER_FORMAT, get_base_dir(), source)
#define TD_JOURNAL_FOLDER(source, account_id) fmt::format(TD_JOURNAL_FOLDER_FORMAT, get_base_dir(), source, account_id)

// commission configuration
#define COMMISSION_DEFAULT_DB_FILE fmt::format("{}/global/commission.db", get_base_dir())
#define COMMISSION_DB_FILE_FORMAT "{}/accounts/{}/commission.db"
#define COMMISSION_DB_FILE(account_id) fmt::format(COMMISSION_DB_FILE_FORMAT, get_base_dir(), account_id)

// account configuration

#define ORDER_DB_FILE_FORMAT "{}/accounts/{}/orders.db"
#define TRADE_DB_FILE_FORMAT "{}/accounts/{}/trades.db"
#define ACCOUNT_ASSET_DB_FILE_FORMAT "{}/accounts/{}/assets.db"
#define ACCOUNT_SNAPSHOT_DB_FILE_FORMAT "{}/accounts/{}/snapshots.db"
#define ACCOUNT_REP_URL_FORMAT "ipc://{}/accounts/{}/rep.ipc"

#define ACCOUNT_FOLDER(account_id) fmt::format("{}/accounts/{}", get_base_dir(), account_id)
#define ORDER_DB_FILE(account_id) fmt::format(ORDER_DB_FILE_FORMAT, get_base_dir(), account_id)
#define TRADE_DB_FILE(account_id) fmt::format(TRADE_DB_FILE_FORMAT, get_base_dir(), account_id)
#define ACCOUNT_ASSET_DB_FILE(account_id) fmt::format(ACCOUNT_ASSET_DB_FILE_FORMAT, get_base_dir(), account_id)
#define ACCOUNT_SNAPSHOT_DB_FILE(account_id) fmt::format(ACCOUNT_SNAPSHOT_DB_FILE_FORMAT, get_base_dir(), account_id)
#define ACCOUNT_REP_URL(account_id) fmt::format(ACCOUNT_REP_URL_FORMAT, get_base_dir(), account_id)

// strategy configuration

#define STRATEGY_MD_FEED_DB_FILE_FORMAT "{}/strategy/{}/md_feed.db"
#define STRATEGY_ACCOUNT_LIST_DB_FILE_FORMAT "{}/strategy/{}/account_list.db"
#define STRATEGY_CURRENT_MD_FEED_DB_FILE_FORMAT "{}/strategy/{}/current_md_feed.db"
#define STRATEGY_CURRENT_ACCOUNT_LIST_DB_FILE_FORMAT "{}/strategy/{}/current_account_list.db"
#define STRATEGY_ASSET_DB_FILE_FORMAT "{}/strategy/{}/assets.db"
#define STRATEGY_SNAPSHOT_DB_FILE_FORMAT "{}/strategy/{}/snapshots.db"

#define STRATEGY_PUB_URL_FORMAT "ipc://{}/strategy/{}/pub.ipc"
#define STRATEGY_REP_URL_FORMAT "ipc://{}/strategy/{}/rep.ipc"

#define STRATEGY_FOLDER(name) fmt::format("{}/strategy/{}", get_base_dir(), name)
#define STRATEGY_MD_FEED_DB_FILE(name) fmt::format(STRATEGY_MD_FEED_DB_FILE_FORMAT, get_base_dir(), name)
#define STRATEGY_ACCOUNT_LIST_DB_FILE(name) fmt::format(STRATEGY_ACCOUNT_LIST_DB_FILE_FORMAT, get_base_dir(), name)
#define STRATEGY_CURRENT_MD_FEED_DB_FILE(name) fmt::format(STRATEGY_CURRENT_MD_FEED_DB_FILE_FORMAT, get_base_dir(), name)
#define STRATEGY_CURRENT_ACCOUNT_LIST_DB_FILE(name) fmt::format(STRATEGY_CURRENT_ACCOUNT_LIST_DB_FILE_FORMAT, get_base_dir(), name)
#define STRATEGY_ASSET_DB_FILE(name) fmt::format(STRATEGY_ASSET_DB_FILE_FORMAT, get_base_dir(), name)
#define STRATEGY_SNAPSHOT_DB_FILE(name) fmt::format(STRATEGY_SNAPSHOT_DB_FILE_FORMAT, get_base_dir(), name)

#define STRATEGY_PUB_URL(name) fmt::format(STRATEGY_PUB_URL_FORMAT, get_base_dir(), name)
#define STRATEGY_REP_URL(name) fmt::format(STRATEGY_REP_URL_FORMAT, get_base_dir(), name)

// algo service configuration

#define ALGO_SERVICE_NAME "algo_service"
#define ALGO_SERVICE_JOURNAL_FOLDER_FORMAT "{}/journal/algo"
#define ALGO_ORDER_DB_FILE_FORMAT "{}/algo/algo_orders.db"
#define ALGO_MD_FEED_DB_FILE_FORMAT "{}/algo/md_feed.db"
#define ALGO_ACCOUNT_LIST_DB_FILE_FORMAT "{}/algo/account_list.db"
#define ALGO_STRATEGY_LIST_DB_FILE_FORMAT "{}/algo/strategy_list.db"

#define ALGO_SERVICE_REP_URL_FORMAT "ipc://{}/algo/rep.ipc"
#define ALGO_SERVICE_PUB_URL_FORMAT "ipc://{}/algo/pub.ipc"

#define ALGO_SERVICE_JOURNAL_FOLDER fmt::format(ALGO_SERVICE_JOURNAL_FOLDER_FORMAT, get_base_dir())
#define ALGO_SERVICE_JOURNAL_NAME "algo"
#define ALGO_ORDER_DB_FILE fmt::format(ALGO_ORDER_DB_FILE_FORMAT, get_base_dir())
#define ALGO_MD_FEED_DB_FILE fmt::format(ALGO_MD_FEED_DB_FILE_FORMAT, get_base_dir())
#define ALGO_ACCOUNT_LIST_DB_FILE fmt::format(ALGO_ACCOUNT_LIST_DB_FILE_FORMAT, get_base_dir())
#define ALGO_STRATEGY_LIST_DB_FILE fmt::format(ALGO_STRATEGY_LIST_DB_FILE_FORMAT, get_base_dir())

#define ALGO_SERVICE_REP_URL fmt::format(ALGO_SERVICE_REP_URL_FORMAT, get_base_dir())
#define ALGO_SERVICE_PUB_URL fmt::format(ALGO_SERVICE_PUB_URL_FORMAT, get_base_dir())

#define SOURCE_CTP "ctp"
#define SOURCE_XTP "xtp"
#define SOURCE_OES "oes"
#define SOURCE_TORA "tora"

#define DAILY_STORAGE_TIME "15:30:00"

#define ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME "trading_account_1m_snapshots"
#define ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME "trading_account_1d_snapshots"

#define PORTFOLIO_ONE_MIN_SNAPSHOT_TABLE_NAME "portfolio_1m_snapshots"
#define PORTFOLIO_ONE_DAY_SNAPSHOT_TABLE_NAME "portfolio_1d_snapshots"

#define RUNTIME_FOLDER fmt::format("{}/runtime", get_base_dir())
}
#endif // PROJECT_CONFIG_H
