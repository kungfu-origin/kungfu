// SPDX-License-Identifier: Apache-2.0

//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_TYPES_H
#define KUNGFU_LONGFIST_TYPES_H

#include <kungfu/common.h>
#include <kungfu/longfist/enums.h>

namespace kungfu::longfist::types {
using namespace kungfu::longfist::enums;

static constexpr int INSTRUMENT_ID_LEN = 32;
static constexpr int ACCOUNT_ID_LEN = 32;
static constexpr int PRODUCT_ID_LEN = 128;
static constexpr int DATE_LEN = 9;
static constexpr int EXCHANGE_ID_LEN = 16;
static constexpr int TRAIDNG_PHASE_CODE_LEN = 8;
static constexpr int ERROR_MSG_LEN = 256;

KF_DEFINE_MARK_TYPE(PageEnd, 10000);
KF_DEFINE_MARK_TYPE(SessionStart, 10001);
KF_DEFINE_MARK_TYPE(SessionEnd, 10002);
KF_DEFINE_MARK_TYPE(BatchOrderBegin, 10016);
KF_DEFINE_MARK_TYPE(BatchOrderEnd, 10017);
KF_DEFINE_MARK_TYPE(Time, 10003);
KF_DEFINE_MARK_TYPE(Ping, 10008);
KF_DEFINE_MARK_TYPE(Pong, 10009);
KF_DEFINE_MARK_TYPE(RequestStop, 10024);
KF_DEFINE_MARK_TYPE(RequestStart, 10025);
KF_DEFINE_MARK_TYPE(CachedReadyToRead, 10060);
KF_DEFINE_MARK_TYPE(RequestCached, 10061);
KF_DEFINE_MARK_TYPE(NewOrderSingle, 353);
KF_DEFINE_MARK_TYPE(CancelOrder, 354);
KF_DEFINE_MARK_TYPE(CancelAllOrder, 355);
KF_DEFINE_MARK_TYPE(BrokerStateRequest, 399);
KF_DEFINE_MARK_TYPE(ResetBookRequest, 400);
KF_DEFINE_MARK_TYPE(MirrorPositionsRequest, 401);
KF_DEFINE_MARK_TYPE(AssetRequest, 402);
KF_DEFINE_MARK_TYPE(PositionRequest, 403);
KF_DEFINE_MARK_TYPE(AssetSync, 404);
KF_DEFINE_MARK_TYPE(PositionSync, 405);
KF_DEFINE_MARK_TYPE(KeepPositionsRequest, 406);
KF_DEFINE_MARK_TYPE(RebuildPositionsRequest, 407);
KF_DEFINE_MARK_TYPE(AlgoOrderInput, 20010);
KF_DEFINE_MARK_TYPE(AlgoOrderReport, 20011);
KF_DEFINE_MARK_TYPE(AlgoOrderModify, 20012);

KF_DEFINE_DATA_TYPE(                              //
    Config, 10005, PK(location_uid), PERPETUAL(), //
    (uint32_t, location_uid),                     //
    (enums::category, category),                  //
    (std::string, group),                         //
    (std::string, name),                          //
    (enums::mode, mode),                          //
    (std::string, value)                          //
);

KF_DEFINE_DATA_TYPE(                                           //
    TimeValue, 20000, PK(update_time), TIMESTAMP(update_time), //
    (int64_t, update_time),                                    //
    (std::string, tag_a),                                      //
    (std::string, tag_b),                                      //
    (std::string, tag_c),                                      //
    (std::string, value)                                       //
);

KF_DEFINE_DATA_TYPE(                                      //
    TimeKeyValue, 20001, PK(key), TIMESTAMP(update_time), //
    (int64_t, update_time),                               //
    (std::string, key),                                   //
    (std::string, tag_a),                                 //
    (std::string, tag_b),                                 //
    (std::string, tag_c),                                 //
    (std::string, value)                                  //
);

KF_DEFINE_DATA_TYPE(                                                     //
    StrategyStateUpdate, 20002, PK(update_time), TIMESTAMP(update_time), //
    (StrategyState, state),                                              //
    (int64_t, update_time),                                              //
    (std::string, info_a),                                               //
    (std::string, info_b),                                               //
    (std::string, info_c),                                               //
    (std::string, value)                                                 //
);

KF_DEFINE_DATA_TYPE(                                   //
    RiskSetting, 10007, PK(location_uid), PERPETUAL(), //
    (uint32_t, location_uid),                          //
    (enums::category, category),                       //
    (std::string, group),                              //
    (std::string, name),                               //
    (enums::mode, mode),                               //
    (std::string, value)                               //
);

KF_DEFINE_DATA_TYPE(                                                     //
    Session, 10010, PK(location_uid, begin_time), TIMESTAMP(begin_time), //
    (uint32_t, location_uid),                                            //
    (enums::category, category),                                         //
    (enums::mode, mode),                                                 //
    (std::string, group),                                                //
    (std::string, name),                                                 //
    (int64_t, begin_time),                                               //
    (int64_t, update_time),                                              //
    (int64_t, end_time),                                                 //
    (uint32_t, frame_count),                                             //
    (uint64_t, data_size)                                                //
);

KF_DEFINE_DATA_TYPE(                                //
    Register, 10011, PK(location_uid), PERPETUAL(), //
    (uint32_t, location_uid),                       //
    (enums::category, category),                    //
    (enums::mode, mode),                            //
    (std::string, group),                           //
    (std::string, name),                            //
    (int32_t, pid),                                 //
    (int64_t, last_active_time),                    //
    (int64_t, checkin_time)                         //
);

KF_DEFINE_DATA_TYPE(                                  //
    Deregister, 10012, PK(location_uid), PERPETUAL(), //
    (uint32_t, location_uid),                         //
    (enums::category, category),                      //
    (enums::mode, mode),                              //
    (std::string, group),                             //
    (std::string, name)                               //
);

KF_DEFINE_PACK_TYPE(                              //
    CacheReset, 10013, PK(msg_type), PERPETUAL(), //
    (int32_t, msg_type)                           //
);

KF_DEFINE_PACK_TYPE(                                  //
    BrokerStateUpdate, 10014, PK(state), PERPETUAL(), //
    (uint32_t, location_uid),                         //
    (BrokerState, state)                              //
);

KF_DEFINE_PACK_TYPE(                                    //
    RequestReadFrom, 10021, PK(source_id), PERPETUAL(), //
    (uint32_t, source_id),                              //
    (int64_t, from_time)                                //
);

KF_DEFINE_PACK_TYPE(                                          //
    RequestReadFromPublic, 10022, PK(source_id), PERPETUAL(), //
    (uint32_t, source_id),                                    //
    (int64_t, from_time)                                      //
);

KF_DEFINE_DATA_TYPE(                                //
    Location, 10026, PK(location_uid), PERPETUAL(), //
    (uint32_t, location_uid),                       //
    (enums::category, category),                    //
    (enums::mode, mode),                            //
    (std::string, group),                           //
    (std::string, name)                             //
);

KF_DEFINE_PACK_TYPE(                                        //
    RequestReadFromSync, 10031, PK(source_id), PERPETUAL(), //
    (uint32_t, source_id),                                  //
    (int64_t, from_time)                                    //
);

KF_DEFINE_PACK_TYPE(                                 //
    RequestWriteTo, 10023, PK(dest_id), PERPETUAL(), //
    (uint32_t, dest_id)                              //
);

KF_DEFINE_PACK_TYPE(                               //
    TradingDay, 10027, PK(timestamp), PERPETUAL(), //
    (int64_t, timestamp)                           //
);

KF_DEFINE_PACK_TYPE(                                     //
    Channel, 10028, PK(source_id, dest_id), PERPETUAL(), //
    (uint32_t, source_id),                               //
    (uint32_t, dest_id)                                  //
);

KF_DEFINE_PACK_TYPE(                                            //
    ChannelRequest, 10029, PK(source_id, dest_id), PERPETUAL(), //
    (uint32_t, source_id),                                      //
    (uint32_t, dest_id)                                         //
);

KF_DEFINE_DATA_TYPE(                                          //
    RequestWriteToBand, 10032, PK(location_uid), PERPETUAL(), //
    (uint32_t, location_uid),                                 //
    (enums::category, category),                              //
    (enums::mode, mode),                                      //
    (std::string, group),                                     //
    (std::string, name)                                       //
);

KF_DEFINE_PACK_TYPE(                                  //
    Band, 10033, PK(source_id, dest_id), PERPETUAL(), //
    (uint32_t, source_id),                            //
    (uint32_t, dest_id)                               //
);

KF_DEFINE_DATA_TYPE(                     //
    Basket, 100040, PK(id), PERPETUAL(), //
    (uint32_t, id),                      //
    (std::string, name),                 //
    (BasketVolumeType, volume_type),     // 比例/数量
    (int64_t, total_volume)              // 总数量
);

KF_DEFINE_PACK_TYPE(                                                                   //
    BasketInstrument, 100041, PK(basket_uid, instrument_id, exchange_id), PERPETUAL(), //
    (uint32_t, basket_uid),                                                            //
    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),                           // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),                               // 交易所ID
    (InstrumentType, instrument_type), // 合约类型                                                // volume比例
    (int64_t, volume),                 // 数量
    (double, rate)                     // 比例
);

KF_DEFINE_PACK_TYPE(                                    //
    RequestCachedDone, 10062, PK(dest_id), PERPETUAL(), //
    (uint32_t, dest_id)                                 //
);

KF_DEFINE_PACK_TYPE(                         //
    TimeRequest, 10004, PK(id), PERPETUAL(), //
    (int32_t, id),                           //
    (int64_t, duration),                     //
    (int64_t, repeat)                        //
);

KF_DEFINE_PACK_TYPE(                                                           //
    TimeReset, 10100, PK(system_clock_count, steady_clock_count), PERPETUAL(), //
    (int64_t, system_clock_count),                                             //
    (int64_t, steady_clock_count)                                              //
);

KF_DEFINE_PACK_TYPE(                                             //
    Commission, 10006, PK(product_id, exchange_id), PERPETUAL(), //
    (kungfu::array<char, PRODUCT_ID_LEN>, product_id),           // 品种
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),         // 交易所

    (InstrumentType, instrument_type), // 合约类型

    (CommissionRateMode, mode), // 手续费模式(按照交易额或者交易量)

    (double, open_ratio),        // 开仓费率
    (double, close_ratio),       // 平仓费率
    (double, close_today_ratio), // 平今费率

    (double, min_commission) // 最小手续费
);

KF_DEFINE_PACK_TYPE(                                              //
    Instrument, 209, PK(instrument_id, exchange_id), PERPETUAL(), //
    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),      // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),          // 交易所ID
    (InstrumentType, instrument_type),                            // 合约类型

    (kungfu::array<int8_t, PRODUCT_ID_LEN>, product_id), // 产品ID (品种)

    (int32_t, contract_multiplier), // 合约乘数
    (double, price_tick),           // 最小变动价位

    (kungfu::array<char, DATE_LEN>, open_date),   // 上市日
    (kungfu::array<char, DATE_LEN>, create_date), // 创建日
    (kungfu::array<char, DATE_LEN>, expire_date), // 到期日

    (int, delivery_year),  // 交割年份
    (int, delivery_month), // 交割月

    (bool, is_trading),         // 当前是否交易
    (bool, force_update_ratio), // 两融柜台折算率及保证金率

    (double, long_margin_ratio),  // 多头保证金率
    (double, short_margin_ratio), // 空头保证金率
    (double, conversion_rate),    // 担保品折扣率
    (double, exchange_rate)       // 汇率
);

KF_DEFINE_PACK_TYPE(                                         //
    InstrumentKey, 210, PK(key), PERPETUAL(),                //
    (uint32_t, key),                                         //
    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所ID
    (InstrumentType, instrument_type)                        // 合约类型
);

KF_DEFINE_DATA_TYPE(                                               //
    CustomSubscribe, 303, PK(update_time), TIMESTAMP(update_time), //
    (int64_t, update_time),                                        //
    (MarketType, market_type),                                     //
    (SubscribeInstrumentType, instrument_type),                    //
    (SubscribeDataType, data_type)                                 //
);

KF_DEFINE_PACK_TYPE(                                         //
    Quote, 101, PK(instrument_id, exchange_id), PERPETUAL(), //
    (kungfu::array<char, DATE_LEN>, trading_day),            // 交易日

    (int64_t, data_time), // 数据生成时间

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所ID

    (InstrumentType, instrument_type), // 合约类型

    (double, pre_close_price),      // 昨收价
    (double, pre_settlement_price), // 昨结价

    (double, last_price), // 最新价
    (int64_t, volume),    // 数量
    (double, turnover),   // 成交金额

    (double, pre_open_interest), // 昨持仓量
    (double, open_interest),     // 持仓量

    (double, open_price), // 今开盘
    (double, high_price), // 最高价
    (double, low_price),  // 最低价

    (double, upper_limit_price), // 涨停板价
    (double, lower_limit_price), // 跌停板价

    (double, close_price),      // 收盘价
    (double, settlement_price), // 结算价
    (double, iopv),             // 基金实时参考净值

    (kungfu::array<double, 10>, bid_price),   // 申买价
    (kungfu::array<double, 10>, ask_price),   // 申卖价
    (kungfu::array<int64_t, 10>, bid_volume), // 申买量
    (kungfu::array<int64_t, 10>, ask_volume), // 申卖量
    (kungfu::array<char, TRAIDNG_PHASE_CODE_LEN>, trading_phase_code)
    // 标的状态, 上交所用四位, 深交所用两位
    //************************************上海现货行情交易状态***************************************************************
    // 该字段为8位字符数组,左起每位表示特定的含义,无定义则填空格。
    // 第0位:‘S’表示启动(开市前)时段,‘C’表示集合竞价时段,‘T’表示连续交易时段,
    // ‘E’表示闭市时段 ,‘P’表示临时停牌,
    // ‘M’表示可恢复交易的熔断(盘中集合竞价),‘N’表示不可恢复交易的熔断(暂停交易至闭市)
    // ‘U’表示收盘集合竞价
    // 第1位:‘0’表示此产品不可正常交易,‘1’表示此产品可正常交易。
    // 第2位:‘0’表示未上市,‘1’表示已上市
    // 第3位:‘0’表示此产品在当前时段不接受进行新订单申报,‘1’ 表示此产品在当前时段可接受进行新订单申报。

    //************************************深圳现货行情交易状态***************************************************************
    // 第 0位:‘S’= 启动(开市前)‘O’= 开盘集合竞价‘T’= 连续竞价‘B’= 休市‘C’= 收盘集合竞价‘E’= 已闭市‘H’= 临时停牌‘A’=
    // 盘后交易‘V’=波动性中断 第 1位:‘0’= 正常状态 ‘1’= 全天停牌
);

KF_DEFINE_PACK_TYPE(                                                    //
    Entrust, 102, PK(instrument_id, exchange_id), TIMESTAMP(data_time), //
    (kungfu::array<char, DATE_LEN>, trading_day),                       // 交易日

    (int64_t, data_time), // 数据生成时间

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所代码

    (InstrumentType, instrument_type), // 合约类型

    (double, price),         // 委托价格
    (int64_t, volume),       // 委托量
    (Side, side),            // 委托方向
    (PriceType, price_type), // 订单价格类型（市价、限价、本方最优）

    (int64_t, main_seq),      // 主序号,
    (int64_t, seq),           // 子序号,
    (int64_t, orig_order_no), // 原始订单号 上海为原始订单号, 深圳为索引号
    (int64_t, biz_index)      // 业务序号
);

KF_DEFINE_PACK_TYPE(                                                        //
    Transaction, 103, PK(instrument_id, exchange_id), TIMESTAMP(data_time), //
    (kungfu::array<char, DATE_LEN>, trading_day),                           // 交易日

    (int64_t, data_time), // 数据生成时间

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所代码

    (InstrumentType, instrument_type), // 合约类型

    (double, price),   // 成交价
    (int64_t, volume), // 成交量

    (int64_t, bid_no), // 买方订单号
    (int64_t, ask_no), // 卖方订单号

    (ExecType, exec_type), // SZ: 成交标识
    (BsFlag, bs_flag),     // 买卖方向

    (int64_t, main_seq), // 主序号
    (int64_t, seq),      // 子序号
    (int64_t, biz_index) // 业务序号
);

KF_DEFINE_PACK_TYPE(                                                 //
    Bar, 110, PK(instrument_id, exchange_id), TIMESTAMP(start_time), //
    (kungfu::array<char, DATE_LEN>, trading_day),                    // 交易日
    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),         // 合约代码
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),             // 交易所代码

    (InstrumentType, instrument_type), // 合约类型

    (int64_t, start_time), // 开始时间
    (int64_t, end_time),   // 结束时间

    (double, open),  // 开
    (double, close), // 收
    (double, low),   // 低
    (double, high),  // 高

    (int64_t, volume),       // 区间交易量
    (int64_t, start_volume), // 初始总交易量

    (int32_t, tick_count) // 区间有效tick数
);

KF_DEFINE_PACK_TYPE(                                       //
    OrderInput, 201, PK(order_id), TIMESTAMP(insert_time), //
    (uint64_t, order_id),                                  // 订单ID
    (uint64_t, parent_id),                                 // 母单号

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约代码
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所代码

    (InstrumentType, instrument_type), // 合约类型

    (double, limit_price),  // 价格
    (double, frozen_price), // 冻结价格

    (int64_t, volume), // 数量

    (bool, is_swap),                     // 互换单
    (Side, side),                        // 买卖方向
    (Offset, offset),                    // 开平方向
    (HedgeFlag, hedge_flag),             // 投机套保标识
    (PriceType, price_type),             // 价格类型
    (VolumeCondition, volume_condition), // 成交量类型
    (TimeCondition, time_condition),     // 成交时间类型
    (uint64_t, block_id),                // 大宗交易信息id, 非大宗交易则为0

    (int64_t, insert_time) // 写入时间
);

KF_DEFINE_PACK_TYPE(                                         //
    BlockMessage, 207, PK(block_id), TIMESTAMP(insert_time), //
    (uint64_t, block_id),      // 大宗交易信息id, 用于TD从OrderInput找到此数据
    (uint32_t, opponent_seat), // 对手方席号
    (uint64_t, match_number),  // 成交约定号
    (bool, is_specific),       // 是否受限(特定)股份
    (int64_t, insert_time)     // 写入时间
);

KF_DEFINE_PACK_TYPE(                                               //
    OrderAction, 202, PK(order_action_id), TIMESTAMP(insert_time), //
    (uint64_t, order_id),                                          // 订单ID
    (uint64_t, order_action_id),                                   // 订单操作ID

    (OrderActionFlag, action_flag), // 订单操作类型

    (double, price),       // 价格
    (int64_t, volume),     // 数量
    (int64_t, insert_time) // 写入时间
);

KF_DEFINE_PACK_TYPE(                                                    //
    OrderActionError, 216, PK(order_action_id), TIMESTAMP(insert_time), //
    (uint64_t, order_id),                                               // 订单ID
    (uint64_t, order_action_id),                                        // 订单操作ID
    (int32_t, error_id),                                                // 错误ID
    (kungfu::array<char, ERROR_MSG_LEN>, error_msg),                    // 错误信息
    (int64_t, insert_time)                                              // 写入时间
);

KF_DEFINE_PACK_TYPE(                                  //
    Order, 203, PK(order_id), TIMESTAMP(insert_time), //
    (uint64_t, order_id),                             // 订单ID
    (uint64_t, external_id),                          // 柜台订单id, 字符型则hash转换
    (uint64_t, parent_id),                            // 母单号

    (int64_t, insert_time), // 订单写入时间
    (int64_t, update_time), // 订单更新时间

    (kungfu::array<char, DATE_LEN>, trading_day), // 交易日

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所ID

    (InstrumentType, instrument_type), // 合约类型

    (double, limit_price),  // 价格
    (double, frozen_price), // 冻结价格, 市价单冻结价格为0

    (int64_t, volume),      // 数量
    (int64_t, volume_left), // 剩余数量

    (double, tax),        // 税
    (double, commission), // 手续费

    (OrderStatus, status), // 订单状态

    (int32_t, error_id),                             // 错误ID
    (kungfu::array<char, ERROR_MSG_LEN>, error_msg), // 错误信息

    (bool, is_swap),                     // 互换单
    (Side, side),                        // 买卖方向
    (Offset, offset),                    // 开平方向
    (HedgeFlag, hedge_flag),             // 投机套保标识
    (PriceType, price_type),             // 价格类型
    (VolumeCondition, volume_condition), // 成交量类型
    (TimeCondition, time_condition)      // 成交时间类型
);

KF_DEFINE_PACK_TYPE(                                         //
    HistoryOrder, 212, PK(order_id), TIMESTAMP(insert_time), //
    (uint64_t, order_id),                                    // 订单ID
    (uint64_t, external_id),                                 // 柜台订单id, 字符型则hash转换

    (int64_t, insert_time), // 订单写入时间
    (int64_t, update_time), // 订单更新时间

    (kungfu::array<char, DATE_LEN>, trading_day), // 交易日

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所ID

    (bool, is_last),              // 是否为本次查询的最后一条记录
    (HistoryDataType, data_type), // 标记本数据是正常数据, 本页最后一条数据, 全部数据的最后一条

    (InstrumentType, instrument_type), // 合约类型

    (double, limit_price),  // 价格
    (double, frozen_price), // 冻结价格, 市价单冻结价格为0

    (int64_t, volume),      // 数量
    (int64_t, volume_left), // 剩余数量

    (double, tax),        // 税
    (double, commission), // 手续费

    (OrderStatus, status), // 订单状态

    (int32_t, error_id),                             // 错误ID
    (kungfu::array<char, ERROR_MSG_LEN>, error_msg), // 错误信息

    (bool, is_swap), // 互换单

    (Side, side),                        // 买卖方向
    (Offset, offset),                    // 开平方向
    (HedgeFlag, hedge_flag),             // 投机套保标识
    (PriceType, price_type),             // 价格类型
    (VolumeCondition, volume_condition), // 成交量类型
    (TimeCondition, time_condition)      // 成交时间类型
);

KF_DEFINE_PACK_TYPE(                                 //
    Trade, 204, PK(trade_id), TIMESTAMP(trade_time), //
    (uint64_t, trade_id),                            // 成交ID

    (uint64_t, order_id),    // 订单ID
    (uint64_t, external_id), // 柜台订单id, 字符型则hash转换

    (int64_t, trade_time),                        // 成交时间
    (kungfu::array<char, DATE_LEN>, trading_day), // 交易日

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所ID

    (InstrumentType, instrument_type), // 合约类型

    (Side, side),            // 买卖方向
    (Offset, offset),        // 开平方向
    (HedgeFlag, hedge_flag), // 投机套保标识

    (double, price),   // 成交价格
    (int64_t, volume), // 成交量

    (double, tax),       // 税
    (double, commission) // 手续费
);

KF_DEFINE_PACK_TYPE(                                        //
    HistoryTrade, 213, PK(trade_id), TIMESTAMP(trade_time), //
    (uint64_t, trade_id),                                   // 成交ID

    (uint64_t, order_id),    // 订单ID
    (uint64_t, external_id), // 柜台订单id, 字符型则hash转换

    (int64_t, trade_time),                        // 成交时间
    (kungfu::array<char, DATE_LEN>, trading_day), // 交易日

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所ID

    (bool, is_last),              // 是否为本次查询的最后一条记录
    (HistoryDataType, data_type), // 标记本数据是正常数据, 本页最后一条数据, 全部数据的最后一条
    (bool, is_withdraw),          // 是否是撤单流水

    (InstrumentType, instrument_type), // 合约类型

    (Side, side),            // 买卖方向
    (Offset, offset),        // 开平方向
    (HedgeFlag, hedge_flag), // 投机套保标识

    (double, price),               // 成交价格
    (int64_t, volume),             // 成交量
    (int64_t, close_today_volume), // 平今日仓量(期货)

    (double, tax),                                  // 税
    (double, commission),                           // 手续费
    (int32_t, error_id),                            // 错误ID
    (kungfu::array<char, ERROR_MSG_LEN>, error_msg) // 错误信息
);

KF_DEFINE_PACK_TYPE(                                                                   //
    Position, 205, PK(holder_uid, instrument_id, exchange_id, direction), PERPETUAL(), //
    (int64_t, update_time),                                                            // 更新时间
    (kungfu::array<char, DATE_LEN>, trading_day),                                      // 交易日

    (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), // 合约ID
    (InstrumentType, instrument_type),                       // 合约类型
    (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     // 交易所ID

    (uint32_t, holder_uid), (LedgerCategory, ledger_category),

    (Direction, direction), // 持仓方向

    (int64_t, volume),           // 数量
    (int64_t, yesterday_volume), // 昨仓数量
    (int64_t, frozen_total),     // 冻结数量
    (int64_t, frozen_yesterday), // 冻结昨仓

    (double, last_price), // 最新价

    (double, avg_open_price),      // 开仓均价
    (double, position_cost_price), // 持仓成本价

    (double, close_price),     // 收盘价(股票和债券)
    (double, pre_close_price), // 昨收价(股票和债券)

    (double, settlement_price),     // 结算价(期货)
    (double, pre_settlement_price), // 昨结算(期货)

    (double, margin),       // 保证金(期货)
    (double, position_pnl), // 持仓盈亏(期货)
    (double, close_pnl),    // 平仓盈亏(期货)

    (double, realized_pnl),  // 已实现盈亏
    (double, unrealized_pnl) // 未实现盈亏
);

KF_DEFINE_PACK_TYPE(                               //
    PositionEnd, 800, PK(holder_uid), PERPETUAL(), //
    (uint32_t, holder_uid)                         //
);

KF_DEFINE_PACK_TYPE(                              //
    Asset, 206, PK(holder_uid), PERPETUAL(),      //
    (int64_t, update_time),                       // 更新时间
    (kungfu::array<char, DATE_LEN>, trading_day), // 交易日

    (uint32_t, holder_uid),            //
    (LedgerCategory, ledger_category), //

    (double, initial_equity), // 期初权益
    (double, static_equity),  // 静态权益
    (double, dynamic_equity), // 动态权益

    (double, realized_pnl), // 累计收益
    (double, unrealized_pnl),

    (double, avail),        // 可用资金
    (double, market_value), // 市值(股票)

    (double, margin), // 保证金(期货)

    (double, accumulated_fee), // 累计手续费
    (double, intraday_fee),    // 当日手续费

    (double, frozen_cash),   // 冻结资金(股票: 买入挂单资金), 期货: 冻结保证金+冻结手续费)
    (double, frozen_margin), // 冻结保证金(期货)
    (double, frozen_fee),    // 冻结手续费(期货)

    (double, position_pnl), // 持仓盈亏(期货)
    (double, close_pnl)     // 平仓盈亏(期货)
);

KF_DEFINE_PACK_TYPE(                               //
    AssetMargin, 211, PK(holder_uid), PERPETUAL(), //
    (int64_t, update_time),                        // 更新时间
    (kungfu::array<char, DATE_LEN>, trading_day),  // 交易日

    (uint32_t, holder_uid),            //
    (LedgerCategory, ledger_category), //

    (double, total_asset),  // 总资产
    (double, avail_margin), // 可用保证金
    (double, cash_margin),  // 融资占用保证金
    (double, short_margin), // 融券占用保证金
    (double, margin),       // 总占用保证金

    (double, cash_debt),  // 融资负债
    (double, short_cash), // 融券卖出金额

    (double, short_market_value),  // 融券卖出证券市值
    (double, margin_market_value), // 融资买入证券市值
    (double, margin_interest),     // 融资融券利息
    (double, settlement),          // 融资融券清算资金

    (double, credit),          // 信贷额度
    (double, collateral_ratio) // 担保比例
);

KF_DEFINE_PACK_TYPE(                                  //
    OrderStat, 215, PK(order_id), TIMESTAMP(md_time), //
    (uint64_t, order_id),                             //
    (int64_t, md_time),                               //
    (int64_t, input_time),                            //
    (int64_t, insert_time),                           //
    (int64_t, ack_time),                              //
    (int64_t, trade_time),                            //
    (double, total_price),                            //
    (double, total_volume),                           //
    (double, avg_price)                               //
);

KF_DEFINE_PACK_TYPE(                                        //
    BasketOrder, 220, PK(order_id), TIMESTAMP(insert_time), //
    (uint64_t, order_id),                                   // 篮子单uid
    (uint64_t, parent_id),                                  // 母单号

    (int64_t, insert_time), // 下单时间
    (int64_t, update_time), // 更新时间

    (Side, side),              // 买卖方向
    (Offset, offset),          // 开平方向
    (PriceType, price_type),   // 价格类型
    (PriceLevel, price_level), // 价格级别
    (double, price_offset),    // 价格偏移量

    (int64_t, volume),      // 成交量
    (int64_t, volume_left), // 剩余数量

    (BasketOrderStatus, status) // 订单状态
);

KF_DEFINE_PACK_TYPE(                                                       //
    RequestHistoryOrder, 10129, PK(trigger_time), TIMESTAMP(trigger_time), //
    (uint64_t, trigger_time),                                              // 触发时间
    (uint32_t, query_num)                                                  // 请求查询的数量
);

KF_DEFINE_PACK_TYPE(                                                       //
    RequestHistoryTrade, 10130, PK(trigger_time), TIMESTAMP(trigger_time), //
    (uint64_t, trigger_time),                                              // 触发时间
    (uint32_t, query_num)                                                  // 请求查询的数量
);

KF_DEFINE_PACK_TYPE(                                                           //
    RequestHistoryOrderError, 10131, PK(trigger_time), TIMESTAMP(insert_time), //
    (int32_t, error_id),                                                       // 错误ID
    (kungfu::array<char, ERROR_MSG_LEN>, error_msg),                           // 错误信息
    (int64_t, trigger_time)                                                    // 写入时间
);

KF_DEFINE_PACK_TYPE(                                                           //
    RequestHistoryTradeError, 10132, PK(trigger_time), TIMESTAMP(insert_time), //
    (int32_t, error_id),                                                       // 错误ID
    (kungfu::array<char, ERROR_MSG_LEN>, error_msg),                           // 错误信息
    (int64_t, trigger_time)                                                    // 写入时间
);

} // namespace kungfu::longfist::types

#endif // KUNGFU_LONGFIST_TYPES_H
