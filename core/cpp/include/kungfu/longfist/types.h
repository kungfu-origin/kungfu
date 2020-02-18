//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_TYPES_H
#define KUNGFU_LONGFIST_TYPES_H

#include <kungfu/common.h>
#include <kungfu/longfist/enums.h>

namespace kungfu::longfist::types
{
    using namespace kungfu::longfist::enums;

    static constexpr int INSTRUMENT_ID_LEN = 32;
    static constexpr int PRODUCT_ID_LEN = 32;
    static constexpr int DATE_LEN = 9;
    static constexpr int EXCHANGE_ID_LEN = 16;
    static constexpr int ACCOUNT_ID_LEN = 32;
    static constexpr int CLIENT_ID_LEN = 32;
    static constexpr int SOURCE_ID_LEN = 16;
    static constexpr int BROKER_ID_LEN = 32;
    static constexpr int ERROR_MSG_LEN = 128;

    KF_DEFINE_MARK_TYPE(PageEnd, 10000);
    KF_DEFINE_MARK_TYPE(SessionStart, 10001);
    KF_DEFINE_MARK_TYPE(SessionEnd, 10002);
    KF_DEFINE_MARK_TYPE(Time, 10003);
    KF_DEFINE_MARK_TYPE(Ping, 10008);
    KF_DEFINE_MARK_TYPE(Pong, 10009);
    KF_DEFINE_MARK_TYPE(RequestStart, 10025);
    KF_DEFINE_MARK_TYPE(Subscribe, 302);
    KF_DEFINE_MARK_TYPE(SubscribeAll, 303);
    KF_DEFINE_MARK_TYPE(NewOrderSingle, 353);
    KF_DEFINE_MARK_TYPE(CancelOrder, 354);
    KF_DEFINE_MARK_TYPE(CancelAllOrder, 355);
    KF_DEFINE_MARK_TYPE(InstrumentRequest, 356);
    KF_DEFINE_MARK_TYPE(BrokerStateUpdate, 401);
    KF_DEFINE_MARK_TYPE(QryAsset, 402);
    KF_DEFINE_MARK_TYPE(InstrumentEnd, 802);
    KF_DEFINE_MARK_TYPE(AlgoOrderInput, 210);
    KF_DEFINE_MARK_TYPE(AlgoOrderReport, 211);
    KF_DEFINE_MARK_TYPE(AlgoOrderModify, 212);

    KF_DEFINE_DATA_TYPE(
            Register, 10011,
            PK(category, group, name, mode),
            (enums::category, category),
            (enums::mode, mode),
            (std::string, group),
            (std::string, name),
            (int32_t, pid)
    );

    KF_DEFINE_DATA_TYPE(
            Deregister, 10012,
            PK(category, group, name, mode),
            (enums::category, category),
            (enums::mode, mode),
            (std::string, group),
            (std::string, name)
    );

    KF_DEFINE_DATA_TYPE(
            Location, 10026,
            PK(category, group, name, mode),
            (enums::category, category),
            (enums::mode, mode),
            (std::string, group),
            (std::string, name)
    );

    KF_DEFINE_PACK_TYPE(
            TimeRequest, 10004,
            PK(id),
            (int32_t, id),
            (int64_t, duration),
            (int64_t, repeat)
    );

    KF_DEFINE_PACK_TYPE(
            RequestReadFrom, 10021,
            PK(source_id),
            (uint32_t, source_id),
            (int64_t, from_time)
    );

    KF_DEFINE_PACK_TYPE(
            RequestReadFromPublic, 10022,
            PK(source_id),
            (uint32_t, source_id),
            (int64_t, from_time)
    );

    KF_DEFINE_PACK_TYPE(
            RequestWriteTo, 10023,
            PK(dest_id),
            (uint32_t, dest_id)
    );

    KF_DEFINE_PACK_TYPE(
            TradingDay, 10027,
            PK(timestamp),
            (int64_t, timestamp)
    );

    KF_DEFINE_PACK_TYPE(
            Channel, 10028,
            PK(source_id, dest_id),
            (uint32_t, source_id),
            (uint32_t, dest_id)
    );

    KF_DEFINE_PACK_TYPE(
            Instrument, 209,
            PK(instrument_id, exchange_id),
            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),     //合约ID
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),         //交易所ID
            (InstrumentType, instrument_type),            //合约类型

            (kungfu::array<char, PRODUCT_ID_LEN>, product_id),           //产品ID

            (int32_t, contract_multiplier),                   //合约乘数
            (double, price_tick),                         //最小变动价位

            (kungfu::array<char, DATE_LEN>, open_date),                  //上市日
            (kungfu::array<char, DATE_LEN>, create_date),                //创建日
            (kungfu::array<char, DATE_LEN>, expire_date),                //到期日

            (int, delivery_year),                         //交割年份
            (int, delivery_month),                        //交割月

            (bool, is_trading),                           //当前是否交易

            (double, long_margin_ratio),                  //多头保证金率
            (double, short_margin_ratio)                 //空头保证金率
    );

    KF_DEFINE_PACK_TYPE(
            Quote, 101,
            PK(instrument_id, exchange_id),
            (kungfu::array<char, SOURCE_ID_LEN>, source_id),              //柜台ID
            (kungfu::array<char, DATE_LEN>, trading_day),                 //交易日

            (int64_t, data_time),                          //数据生成时间

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),      //合约ID
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),          //交易所ID

            (InstrumentType, instrument_type),             //合约类型

            (double, pre_close_price),                     //昨收价
            (double, pre_settlement_price),                //昨结价

            (double, last_price),                          //最新价
            (int64_t, volume),                             //数量
            (double, turnover),                            //成交金额

            (double, pre_open_interest),                   //昨持仓量
            (double, open_interest),                       //持仓量

            (double, open_price),                          //今开盘
            (double, high_price),                          //最高价
            (double, low_price),                           //最低价

            (double, upper_limit_price),                   //涨停板价
            (double, lower_limit_price),                   //跌停板价

            (double, close_price),                         //收盘价
            (double, settlement_price),                    //结算价

            (kungfu::array<double, 10>, bid_price),                       //申买价
            (kungfu::array<double, 10>, ask_price),                       //申卖价
            (kungfu::array<int64_t, 10>, bid_volume),                     //申买量
            (kungfu::array<int64_t, 10>, ask_volume)                     //申卖量
    );

    KF_DEFINE_PACK_TYPE(
            Entrust, 102,
            PK(instrument_id, exchange_id),
            (kungfu::array<char, SOURCE_ID_LEN>, source_id),              //柜台ID
            (kungfu::array<char, DATE_LEN>, trading_day),                 //交易日

            (int64_t, data_time),                          //数据生成时间

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),      //合约ID
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),          //交易所代码

            (InstrumentType, instrument_type),             //合约类型

            (double, price),                               //委托价格
            (int64_t, volume),                             //委托量
            (Side, side),                                  //委托方向
            (PriceType, price_type),                       //订单价格类型（市价、限价、本方最优）

            (int64_t, main_seq),                           //主序号
            (int64_t, seq)                                //子序号
    );

    KF_DEFINE_PACK_TYPE(
            Transaction, 103,
            PK(instrument_id, exchange_id),
            (kungfu::array<char, SOURCE_ID_LEN>, source_id),              //柜台ID
            (kungfu::array<char, DATE_LEN>, trading_day),                 //交易日

            (int64_t, data_time),                          //数据生成时间

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),      //合约ID
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),          //交易所代码

            (InstrumentType, instrument_type),             //合约类型

            (double, price),                               //成交价
            (int64_t, volume),                             //成交量

            (int64_t, bid_no),                             //买方订单号
            (int64_t, ask_no),                             //卖方订单号

            (ExecType, exec_type),                         //SZ: 成交标识
            (BsFlag, bs_flag),                            //SH: 内外盘标识

            (int64_t, main_seq),                               //主序号
            (int64_t, seq)                                    //子序号
    );

    KF_DEFINE_PACK_TYPE(
            Bar, 110,
            PK(instrument_id, exchange_id),
            (kungfu::array<char, DATE_LEN>, trading_day),            //交易日
            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), //合约代码
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     //交易所代码

            (int64_t, start_time),                    //开始时间
            (int64_t, end_time),                      //结束时间

            (double, open),                           //开
            (double, close),                          //收
            (double, low),                            //低
            (double, high),                           //高

            (int64_t, volume),                        //区间交易量
            (int64_t, start_volume),                  //初始总交易量

            (int32_t, tick_count)                    //区间有效tick数
    );

    KF_DEFINE_PACK_TYPE(
            OrderInput, 201,
            PK(order_id),
            (uint64_t, order_id),                       //订单ID

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),   //合约代码
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),       //交易所代码

            (kungfu::array<char, SOURCE_ID_LEN>, source_id),           //柜台ID
            (kungfu::array<char, ACCOUNT_ID_LEN>, account_id),         //账号ID

            (InstrumentType, instrument_type),          //合约类型

            (double, limit_price),                      //价格
            (double, frozen_price),                     //冻结价格

            (int64_t, volume),                          //数量

            (Side, side),                               //买卖方向
            (Offset, offset),                           //开平方向
            (HedgeFlag, hedge_flag),                    //投机套保标识
            (PriceType, price_type),                    //价格类型
            (VolumeCondition, volume_condition),        //成交量类型
            (TimeCondition, time_condition),            //成交时间类型

            (uint64_t, parent_id)                      //母订单ID
    );

    KF_DEFINE_PACK_TYPE(
            OrderAction, 202,
            PK(order_action_id),
            (uint64_t, order_id),                       //订单ID
            (uint64_t, order_action_id),                //订单操作ID

            (OrderActionFlag, action_flag),             //订单操作类型

            (double, price),                            //价格
            (int64_t, volume)                          //数量
    );

    KF_DEFINE_PACK_TYPE(
            OrderActionError, 202,
            PK(order_action_id),
            (uint64_t, order_id),                       //订单ID
            (uint64_t, order_action_id),                //订单操作ID
            (int32_t, error_id),                        //错误ID
            (kungfu::array<char, ERROR_MSG_LEN>, error_msg)           //错误信息
    );

    KF_DEFINE_PACK_TYPE(
            Order, 203,
            PK(order_id),
            (uint64_t, parent_id),                      //母订单ID
            (uint64_t, order_id),                       //订单ID

            (int64_t, insert_time),                     //订单写入时间
            (int64_t, update_time),                     //订单更新时间

            (kungfu::array<char, DATE_LEN>, trading_day),              //交易日

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),   //合约ID
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),       //交易所ID

            (kungfu::array<char, SOURCE_ID_LEN>, source_id),           //柜台ID
            (kungfu::array<char, ACCOUNT_ID_LEN>, account_id),         //账号ID
            (kungfu::array<char, CLIENT_ID_LEN>, client_id),           //Client ID

            (InstrumentType, instrument_type),          //合约类型

            (double, limit_price),                      //价格
            (double, frozen_price),                     //冻结价格，市价单冻结价格为0

            (int64_t, volume),                          //数量
            (int64_t, volume_traded),                   //成交数量
            (int64_t, volume_left),                     //剩余数量

            (double, tax),                              //税
            (double, commission),                       //手续费

            (OrderStatus, status),                      //订单状态

            (int32_t, error_id),                        //错误ID
            (kungfu::array<char, ERROR_MSG_LEN>, error_msg),           //错误信息

            (Side, side),                               //买卖方向
            (Offset, offset),                           //开平方向
            (HedgeFlag, hedge_flag),                    //投机套保标识
            (PriceType, price_type),                    //价格类型
            (VolumeCondition, volume_condition),        //成交量类型
            (TimeCondition, time_condition)            //成交时间类型
    );

    KF_DEFINE_PACK_TYPE(
            Trade, 204,
            PK(order_id),
            (uint64_t, trade_id),                       //成交ID

            (uint64_t, order_id),                       //订单ID
            (uint64_t, parent_order_id),                //母订单ID

            (int64_t, trade_time),                     //成交时间
            (kungfu::array<char, DATE_LEN>, trading_day),              //交易日

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),   //合约ID
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),       //交易所ID
            (kungfu::array<char, SOURCE_ID_LEN>, source_id),           //柜台ID
            (kungfu::array<char, ACCOUNT_ID_LEN>, account_id),         //账号ID
            (kungfu::array<char, CLIENT_ID_LEN>, client_id),           //Client ID

            (InstrumentType, instrument_type),          //合约类型

            (Side, side),                               //买卖方向
            (Offset, offset),                           //开平方向
            (HedgeFlag, hedge_flag),                    //投机套保标识

            (double, price),                            //成交价格
            (int64_t, volume),                          //成交量
            (int64_t, close_today_volume),              //平今日仓量(期货)

            (double, tax),                              //税
            (double, commission)                       //手续费
    );

    KF_DEFINE_PACK_TYPE(
            Position, 205,
            PK(holder_uid, instrument_id, exchange_id, direction),
            (int64_t, update_time),                     //更新时间
            (kungfu::array<char, DATE_LEN>, trading_day),              //交易日

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),   //合约ID
            (InstrumentType, instrument_type),          //合约类型
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),       //交易所ID

            (uint32_t, holder_uid),
            (LedgerCategory, ledger_category),

            (kungfu::array<char, SOURCE_ID_LEN>, source_id),           //柜台ID
            (kungfu::array<char, ACCOUNT_ID_LEN>, account_id),         //账号ID
            (kungfu::array<char, CLIENT_ID_LEN>, client_id),           //Client ID

            (Direction, direction),                     //持仓方向

            (int64_t, volume),                          //数量
            (int64_t, yesterday_volume),                //昨仓数量
            (int64_t, frozen_total),                    //冻结数量
            (int64_t, frozen_yesterday),                //冻结昨仓

            (double, last_price),                       //最新价

            (double, avg_open_price),                   //开仓均价
            (double, position_cost_price),              //持仓成本价

            (double, close_price),                      //收盘价(股票和债券)
            (double, pre_close_price),                  //昨收价(股票和债券)

            (double, settlement_price),                 //结算价(期货)
            (double, pre_settlement_price),             //昨结算(期货)

            (double, margin),                           //保证金(期货)
            (double, position_pnl),                     //持仓盈亏(期货)
            (double, close_pnl),                        //平仓盈亏(期货)

            (double, realized_pnl),                     //已实现盈亏
            (double, unrealized_pnl)                   //未实现盈亏
    );

    KF_DEFINE_PACK_TYPE(
            PositionEnd, 800,
            PK(holder_uid),
            (uint32_t, holder_uid)
    );

    KF_DEFINE_PACK_TYPE(
            Asset, 206,
            PK(holder_uid),
            (int64_t, update_time),               //更新时间
            (kungfu::array<char, DATE_LEN>, trading_day),        //交易日

            (uint32_t, holder_uid),
            (LedgerCategory, ledger_category),

            (kungfu::array<char, SOURCE_ID_LEN>, source_id),     //柜台ID
            (kungfu::array<char, BROKER_ID_LEN>, broker_id),     //Broker ID
            (kungfu::array<char, ACCOUNT_ID_LEN>, account_id),   //账号ID
            (kungfu::array<char, CLIENT_ID_LEN>, client_id),     //client ID

            (double, initial_equity),             //期初权益
            (double, static_equity),              //静态权益
            (double, dynamic_equity),             //动态权益

            (double, realized_pnl),               //累计收益
            (double, unrealized_pnl),

            (double, avail),                      //可用资金
            (double, market_value),               //市值(股票)

            (double, margin),                     //保证金(期货)

            (double, accumulated_fee),            //累计手续费
            (double, intraday_fee),               //当日手续费

            (double, frozen_cash),                //冻结资金(股票: 买入挂单资金), 期货: 冻结保证金+冻结手续费)
            (double, frozen_margin),              //冻结保证金(期货)
            (double, frozen_fee),                 //冻结手续费(期货)

            (double, position_pnl),               //持仓盈亏(期货)
            (double, close_pnl)                  //平仓盈亏(期货)
    );

    KF_DEFINE_PACK_TYPE(
            PositionDetail, 208,
            PK(instrument_id, exchange_id, source_id, account_id),
            (int64_t, update_time),                     //更新时间
            (kungfu::array<char, DATE_LEN>, trading_day),              //交易日

            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id),   //合约ID
            (InstrumentType, instrument_type),          //合约类型
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),       //交易所ID

            (uint32_t, holder_uid),

            (kungfu::array<char, SOURCE_ID_LEN>, source_id),           //柜台ID
            (kungfu::array<char, ACCOUNT_ID_LEN>, account_id),         //账号ID
            (kungfu::array<char, CLIENT_ID_LEN>, client_id),           //Client ID

            (Direction, direction),                     //持仓方向

            (int64_t, volume),                          //数量
            (int64_t, frozen_volume),                   //冻结数量

            (double, last_price),                       //最新价
            (double, open_price),                       //开仓价

            (double, settlement_price),                 //结算价(期货)
            (double, pre_settlement_price),             //昨结算(期货)

            (kungfu::array<char, DATE_LEN>, open_date),                //开仓日(YYYYMMDD,仅期货明细和债券)

            (uint64_t, trade_id),                       //成交ID
            (int64_t, trade_time)                      //成交时间
    );

    KF_DEFINE_PACK_TYPE(
            PositionDetailEnd, 801,
            PK(holder_uid),
            (uint32_t, holder_uid)
    );

    KF_DEFINE_PACK_TYPE(
            InstrumentCommissionRate, 214,
            PK(instrument_id, exchange_id),
            (kungfu::array<char, INSTRUMENT_ID_LEN>, instrument_id), //合约代码
            (kungfu::array<char, EXCHANGE_ID_LEN>, exchange_id),     //交易所代码


            (InstrumentType, instrument_type),         //合约类型

            (kungfu::array<char, ACCOUNT_ID_LEN>, account_id),        //账户ID
            (kungfu::array<char, BROKER_ID_LEN>, broker_id),          //Broker ID

            (CommissionRateMode, mode),                //手续费模式(按照交易额或者交易量)

            (double, open_ratio),                      //开仓费率
            (double, close_ratio),                     //平仓费率
            (double, close_today_ratio),               //平今费率

            (double, min_commission)                  //最小手续费
    );

    KF_DEFINE_DATA_TYPE(
            Config, 10005,
            PK(name),
            (std::string, name),
            (std::string, value)
    );

    KF_DEFINE_DATA_TYPE(
            OrderStat, 215,
            PK(order_id),
            (uint64_t, order_id),
            (int64_t, md_time),
            (int64_t, insert_time),
            (int64_t, ack_time),
            (std::vector<int64_t>, timestamps)
    );
}

#endif //KUNGFU_LONGFIST_TYPES_H
