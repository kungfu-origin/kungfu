//
// Created by qlu on 2019/3/9.
//

#ifndef KUNGFU_MACRO_H
#define KUNGFU_MACRO_H

#include <spdlog/spdlog.h>

#define INSERT_ORDER_TRACE(content) SPDLOG_TRACE("[InsertOrder] {}", content)
#define INSERT_ORDER_ERROR(content) SPDLOG_ERROR("[InsertOrder] {}", content)
#define INSERT_ORDER_INFO(content)  SPDLOG_INFO("[InsertOrder] {}", content)

#define CANCEL_ORDER_TRACE(content) SPDLOG_TRACE("[CancelOrder] {}", content)
#define CANCEL_ORDER_ERROR(content) SPDLOG_ERROR("[CancelOrder] {}", content)
#define CANCEL_ORDER_INFO(content)  SPDLOG_INFO("[CancelOrder] {}", content)

#define ORDER_TRACE(content) SPDLOG_TRACE("[Order] {}", content)
#define ORDER_ERROR(content) SPDLOG_ERROR("[Order] {}", content)
#define ORDER_INFO(content)  SPDLOG_INFO("[Order] {}", content)

#define TRADE_TRACE(content) SPDLOG_TRACE("[Trade] {}", content)
#define TRADE_ERROR(content) SPDLOG_ERROR("[Trade] {}", content)
#define TRADE_INFO(content)  SPDLOG_INFO("[Trade] {}", content)

#define LOGIN_TRACE(content) SPDLOG_TRACE("[Login] {}", content)
#define LOGIN_INFO(content) SPDLOG_INFO("[Login] {}",content)
#define LOGIN_ERROR(content) SPDLOG_ERROR("[Login] {}", content)

#define CONNECT_INFO() SPDLOG_INFO("[Connected]")
#define DISCONNECTED_ERROR(content) SPDLOG_ERROR("[DisConnected] {}", content)

#define ACCOUNT_TRACE(content) SPDLOG_TRACE("[Account] {}", content)
#define ACCOUNT_INFO(content) SPDLOG_INFO("[Account] {}", content)
#define ACCOUNT_ERROR(content)  SPDLOG_ERROR("[Account] {}", content)

#define POSITION_TRACE(content) SPDLOG_TRACE("[Position] {}", content)
#define POSITION_INFO(content) SPDLOG_INFO("[Position] {}", content)
#define POSITION_ERROR(content)  SPDLOG_ERROR("[Position] {}", content)

#define POSITION_DETAIL_TRACE(content) SPDLOG_TRACE("[PositionDetail] {}", content)
#define POSITION_DETAIL_ERROR(content)  SPDLOG_ERROR("[PositionDetail] {}", content)

#define INSTRUMENT_TRACE(content) SPDLOG_TRACE("[Instrument] {}", content)
#define INSTRUMENT_ERROR(content) SPDLOG_ERROR("[Instrument] {}", content)

#define SUBSCRIBE_TRACE(content) SPDLOG_TRACE("[Subscribe] {}" , content)
#define SUBSCRIBE_INFO(content) SPDLOG_INFO("[Subscribe] {}", content)
#define SUBSCRIBE_ERROR(content) SPDLOG_ERROR("[Subscribe] {}", content)

#define UNSUBSCRIBE_TRACE(content) SPDLOG_TRACE("[Unsubscribe] {}" , content)
#define UNSUBSCRIBE_INFO(content) SPDLOG_INFO("[Unsubscribe] {}", content)
#define UNSUBSCRIBE_ERROR(content) SPDLOG_ERROR("[Unsubscribe] {}", content)

#define QUOTE_TRACE(content)  SPDLOG_TRACE("[Quote] {}", content)
#define ENTRUST_TRACE(content)  SPDLOG_TRACE("[Entrust] {}", content)
#define TRANSACTION_TRACE(content) SPDLOG_TRACE("[Transaction] {}", content)

#define TIMER_TRACE(content) SPDLOG_TRACE("[Timer] {}", content)

#endif //KUNGFU_MACRO_H
