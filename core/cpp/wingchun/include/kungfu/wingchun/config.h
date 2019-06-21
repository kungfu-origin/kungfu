//
// Created by qlu on 2019/2/15.
//

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

namespace kungfu
{
// uid generator configuration
#define UID_WORKER_DB_FILE_FORMAT "{}/global/uid_worker.db"

#define UID_WORKER_DB_FILE fmt::format(UID_WORKER_DB_FILE_FORMAT, get_kungfu_home())

// future instrument info configuration
#define FUTURE_INSTRUMENT_DB_FILE_FORMAT "{}/global/future_instrument.db"

// calendar configuration
#define CALENDAR_HOLIDAY_DB_FILE_FORMAT "{}/global/holidays.db"

// gateway configuration

// commission configuration
#define COMMISSION_DEFAULT_DB_FILE fmt::format("{}/global/commission.db", get_kungfu_home())
#define COMMISSION_DB_FILE_FORMAT "{}/accounts/{}/commission.db"
#define COMMISSION_DB_FILE(account_id) fmt::format(COMMISSION_DB_FILE_FORMAT, get_kungfu_home(), account_id)

// account configuration

#define ACCOUNT_FOLDER(account_id) fmt::format("{}/accounts/{}", get_kungfu_home(), account_id)

// strategy configuration

#define STRATEGY_MD_FEED_DB_FILE_FORMAT "{}/strategy/{}/md_feed.db"
#define STRATEGY_ACCOUNT_LIST_DB_FILE_FORMAT "{}/strategy/{}/account_list.db"
#define STRATEGY_ASSET_DB_FILE_FORMAT "{}/strategy/{}/assets.db"
#define STRATEGY_SNAPSHOT_DB_FILE_FORMAT "{}/strategy/{}/snapshots.db"

#define STRATEGY_FOLDER(name) fmt::format("{}/strategy/{}", get_kungfu_home(), name)
#define STRATEGY_MD_FEED_DB_FILE(name) fmt::format(STRATEGY_MD_FEED_DB_FILE_FORMAT, get_kungfu_home(), name)
#define STRATEGY_ACCOUNT_LIST_DB_FILE(name) fmt::format(STRATEGY_ACCOUNT_LIST_DB_FILE_FORMAT, get_kungfu_home(), name)
#define STRATEGY_ASSET_DB_FILE(name) fmt::format(STRATEGY_ASSET_DB_FILE_FORMAT, get_kungfu_home(), name)
#define STRATEGY_SNAPSHOT_DB_FILE(name) fmt::format(STRATEGY_SNAPSHOT_DB_FILE_FORMAT, get_kungfu_home(), name)

#define SOURCE_CTP "ctp"
#define SOURCE_XTP "xtp"

#define ACCOUNT_ONE_MIN_SNAPSHOT_TABLE_NAME "trading_account_1m_snapshots"
#define ACCOUNT_ONE_DAY_SNAPSHOT_TABLE_NAME "trading_account_1d_snapshots"

#define PORTFOLIO_ONE_MIN_SNAPSHOT_TABLE_NAME "portfolio_1m_snapshots"
#define PORTFOLIO_ONE_DAY_SNAPSHOT_TABLE_NAME "portfolio_1d_snapshots"
}
#endif // PROJECT_CONFIG_H
