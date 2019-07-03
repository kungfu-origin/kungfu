//
// Created by Keren Dong on 2019-06-19.
//

#ifndef WINGCHUN_EVENT_H
#define WINGCHUN_EVENT_H

#include <cinttypes>
#include <cmath>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include <kungfu/wingchun/common.h>

#define FORMAT_DOUBLE(x) rounded(x, 4)

namespace kungfu
{
    namespace wingchun
    {
        namespace msg
        {
            namespace type
            {
                enum MsgType
                {
                    Error = 0,

                    Quote = 101,
                    Entrust = 102,
                    Transaction = 103,

                    OrderInput = 201,
                    OrderAction = 202,
                    Order = 203,
                    Trade = 204,
                    Position = 205,
                    AccountInfo = 206,
                    Portfolio = 207,
                    AccountInfoByMin = 208,
                    PortfolioByMin = 209,
                    PositionDetail = 210,
                    SubPortfolioInfo = 211,


                    ReqLogin = 301,
                    Subscribe = 302,
                    ReqOrderInput = 303,
                    ReqOrderAction = 304,

                    RspLogin = 351,
                    RspSubscribe = 352,
                    RspOrderInput = 353,
                    RspOrderAction = 354,

                    GatewayState = 401,

                    AlgoOrderInput = 501,
                    AlgoOrderStatus = 502,
                    AlgoOrderAction = 503,

                    SwitchDay = 601,
                    RspTradingDay = 602,

                    ReloadFutureInstrument = 701,

                    PositionEnd = 800,
                    PositionDetailEnd = 801,
                };
            }

            namespace data
            {
                enum class GatewayState : int
                {
                    Unknown = 0,
                    Idle = 1,
                    DisConnected = 2,
                    Connected = 3,
                    LoggedIn = 4,
                    LoggedInFailed = 5,
                    SettlementConfirmed = 6,
                    SettlementConfirmFailed = 7,
                    AccountInfoConfirmed = 8,
                    AccountInfoConfirmFailed = 9,
                    PositionInfoConfirmed = 10,
                    PositionInfoConfirmFailed = 11,
                    InstrumentInfoConfirmed = 12,
                    InstrumentInfoConfirmFailed = 13,
                    PositionDetailConfirmed = 14,
                    PositionDetailConfirmFailed = 15,
                    Ready = 100
                };

                //合约信息
                struct Instrument
                {
                    char instrument_id[INSTRUMENT_ID_LEN];     //合约ID
                    char exchange_id[EXCHANGE_ID_LEN];         //交易所ID
                    InstrumentType instrument_type;            //合约类型

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                typedef Instrument StockInstrument;

                inline void from_json(const nlohmann::json &j, Instrument &instrument)
                {
                    strcpy(instrument.exchange_id, j["exchange_id"].get<std::string>().c_str());
                    strcpy(instrument.instrument_id, j["instrument_id"].get<std::string>().c_str());
                    instrument.instrument_type = j["instrument_type"];
                }

                inline void to_json(nlohmann::json &j, const Instrument &instrument)
                {
                    j["exchange_id"] = std::string(instrument.exchange_id);
                    j["instrument_id"] = std::string(instrument.instrument_id);
                    j["instrument_type"] = instrument.instrument_type;
                }

                //期货合约信息
                struct FutureInstrument : Instrument
                {
                    char product_id[PRODUCT_ID_LEN];           //产品ID

                    int contract_multiplier;                   //合约乘数
                    double price_tick;                         //最小变动价位

                    char open_date[DATE_LEN];                  //上市日
                    char create_date[DATE_LEN];                //创建日
                    char expire_date[DATE_LEN];                //到期日

                    int delivery_year;                         //交割年份
                    int delivery_month;                        //交割月

                    bool is_trading;                           //当前是否交易

                    double long_margin_ratio;                  //多头保证金率
                    double short_margin_ratio;                 //空头保证金率

                    std::string get_product_id()
                    { return std::string(product_id); }

                    std::string get_open_date()
                    { return std::string(open_date); }

                    std::string get_create_date()
                    { return std::string(create_date); }

                    std::string get_expire_date()
                    { return std::string(expire_date); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const FutureInstrument &instrument)
                {
                    j["exchange_id"] = std::string(instrument.exchange_id);
                    j["instrument_id"] = std::string(instrument.instrument_id);
                    j["instrument_type"] = instrument.instrument_type;
                    j["product_id"] = std::string(instrument.product_id);
                    j["contract_multiplier"] = instrument.contract_multiplier;
                    j["price_tick"] = instrument.price_tick;
                    j["open_date"] = std::string(instrument.open_date);
                    j["create_date"] = std::string(instrument.create_date);
                    j["expire_date"] = std::string(instrument.expire_date);
                    j["delivery_year"] = instrument.delivery_year;
                    j["delivery_month"] = instrument.delivery_month;
                    j["long_margin_ratio"] = instrument.long_margin_ratio;
                    j["short_margin_ratio"] = instrument.short_margin_ratio;
                }

                //行情
                struct Quote
                {
                    char source_id[SOURCE_ID_LEN];              //柜台ID
                    char trading_day[DATE_LEN];                 //交易日

                    int64_t data_time;                          //数据生成时间

                    char instrument_id[INSTRUMENT_ID_LEN];      //合约ID
                    char exchange_id[EXCHANGE_ID_LEN];          //交易所ID

                    InstrumentType instrument_type;             //合约类型

                    double pre_close_price;                     //昨收价
                    double pre_settlement_price;                //昨结价

                    double last_price;                          //最新价
                    int64_t volume;                             //数量
                    double turnover;                            //成交金额

                    double pre_open_interest;                   //昨持仓量
                    double open_interest;                       //持仓量

                    double open_price;                          //今开盘
                    double high_price;                          //最高价
                    double low_price;                           //最低价

                    double upper_limit_price;                   //涨停板价
                    double lower_limit_price;                   //跌停板价

                    double close_price;                         //收盘价
                    double settlement_price;                    //结算价

                    double bid_price[10];                       //申买价
                    double ask_price[10];                       //申卖价
                    int64_t bid_volume[10];                     //申买量
                    int64_t ask_volume[10];                     //申卖量

                    std::string get_source_id()
                    { return std::string(source_id); }

                    std::string get_trading_day()
                    { return std::string(trading_day); }

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

                    std::vector<double> get_bid_price()
                    { return std::vector<double>(bid_price, bid_price + 10); }

                    std::vector<double> get_ask_price()
                    { return std::vector<double>(ask_price, ask_price + 10); }

                    std::vector<int64_t> get_bid_volume()
                    { return std::vector<int64_t>(bid_volume, bid_volume + 10); }

                    std::vector<int64_t> get_ask_volume()
                    { return std::vector<int64_t>(ask_volume, ask_volume + 10); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Quote &quote)
                {
                    j["trading_day"] = std::string(quote.trading_day);
                    j["data_time"] = quote.data_time;
                    j["instrument_id"] = std::string(quote.instrument_id);
                    j["exchange_id"] = std::string(quote.exchange_id);
                    j["instrument_type"] = quote.instrument_type;

                    j["pre_close_price"] = FORMAT_DOUBLE(quote.pre_close_price);
                    j["pre_settlement_price"] = FORMAT_DOUBLE(quote.pre_settlement_price);

                    j["last_price"] = FORMAT_DOUBLE(quote.last_price);
                    j["volume"] = quote.volume;
                    j["turnover"] = FORMAT_DOUBLE(quote.turnover);

                    j["pre_open_interest"] = FORMAT_DOUBLE(quote.pre_open_interest);
                    j["open_interest"] = FORMAT_DOUBLE(quote.open_interest);

                    j["open_price"] = FORMAT_DOUBLE(quote.open_price);
                    j["high_price"] = FORMAT_DOUBLE(quote.high_price);
                    j["low_price"] = FORMAT_DOUBLE(quote.low_price);

                    j["upper_limit_price"] = FORMAT_DOUBLE(quote.upper_limit_price);
                    j["lower_limit_price"] = FORMAT_DOUBLE(quote.lower_limit_price);

                    j["close_price"] = FORMAT_DOUBLE(quote.close_price);
                    j["settlement_price"] = FORMAT_DOUBLE(quote.settlement_price);

                    j["bid_price"] = std::vector<double>(quote.bid_price, std::end(quote.bid_price));
                    j["ask_price"] = std::vector<double>(quote.ask_price, std::end(quote.ask_price));

                    j["bid_volume"] = std::vector<int64_t>(quote.bid_volume, std::end(quote.bid_volume));
                    j["ask_volume"] = std::vector<int64_t>(quote.ask_volume, std::end(quote.ask_volume));
                }

                //逐笔委托
                struct Entrust
                {
                    char source_id[SOURCE_ID_LEN];              //柜台ID
                    char trading_day[DATE_LEN];                 //交易日

                    int64_t data_time;                          //数据生成时间

                    char instrument_id[INSTRUMENT_ID_LEN];      //合约ID
                    char exchange_id[EXCHANGE_ID_LEN];          //交易所代码

                    InstrumentType instrument_type;             //合约类型

                    double price;                               //委托价格
                    int64_t volume;                             //委托量
                    Side side;                                  //委托方向
                    PriceType price_type;                       //订单价格类型（市价、限价、本方最优）

                    int64_t main_seq;                           //主序号
                    int64_t seq;                                //子序号

                    std::string get_source_id()
                    { return std::string(source_id); }

                    std::string get_trading_day()
                    { return std::string(trading_day); }

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Entrust &entrust)
                {
                    j["source_id"] = std::string(entrust.source_id);
                    j["trading_day"] = std::string(entrust.trading_day);
                    j["data_time"] = entrust.data_time;
                    j["instrument_id"] = std::string(entrust.instrument_id);
                    j["exchange_id"] = std::string(entrust.exchange_id);
                    j["instrument_type"] = entrust.instrument_type;
                    j["price"] = FORMAT_DOUBLE(entrust.price);
                    j["volume"] = entrust.volume;
                    j["side"] = entrust.side;
                    j["price_type"] = entrust.price_type;
                    j["main_seq"] = entrust.main_seq;
                    j["seq"] = entrust.seq;
                }

                //逐笔成交
                struct Transaction
                {
                    char source_id[SOURCE_ID_LEN];              //柜台ID
                    char trading_day[DATE_LEN];                 //交易日

                    int64_t data_time;                          //数据生成时间

                    char instrument_id[INSTRUMENT_ID_LEN];      //合约ID
                    char exchange_id[EXCHANGE_ID_LEN];          //交易所代码

                    InstrumentType instrument_type;             //合约类型

                    double price;                               //成交价
                    int64_t volume;                             //成交量

                    int64_t bid_no;                             //买方订单号
                    int64_t ask_no;                             //卖方订单号

                    ExecType exec_type;                         //SZ: 成交标识
                    BsFlag bs_flag;                            //SH: 内外盘标识

                    int64_t main_seq;                               //主序号
                    int64_t seq;                                    //子序号

                    std::string get_source_id()
                    { return std::string(source_id); }

                    std::string get_trading_day()
                    { return std::string(trading_day); }

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Transaction &transaction)
                {
                    j["source_id"] = std::string(transaction.source_id);
                    j["trading_day"] = std::string(transaction.trading_day);
                    j["data_time"] = transaction.data_time;
                    j["instrument_id"] = std::string(transaction.instrument_id);
                    j["exchange_id"] = std::string(transaction.exchange_id);
                    j["instrument_type"] = transaction.instrument_type;
                    j["price"] = FORMAT_DOUBLE(transaction.price);
                    j["volume"] = transaction.volume;
                    j["bid_no"] = transaction.bid_no;
                    j["ask_no"] = transaction.ask_no;
                    j["exec_type"] = transaction.exec_type;
                    j["bs_flag"] = transaction.bs_flag;
                    j["main_seq"] = transaction.main_seq;
                    j["seq"] = transaction.seq;
                }


                //合约手续费率
                struct InstrumentCommissionRate
                {
                    char instrument_id[INSTRUMENT_ID_LEN];  //合约代码
                    char exchange_id[EXCHANGE_ID_LEN];      //交易所代码

                    InstrumentType instrument_type;         //合约类型

                    char account_id[ACCOUNT_ID_LEN];        //账户ID
                    char broker_id[BROKER_ID_LEN];          //Broker ID

                    CommissionRateMode mode;                //手续费模式(按照交易额或者交易量)

                    double open_ratio;                      //开仓费率
                    double close_ratio;                     //平仓费率
                    double close_today_ratio;               //平今费率

                    double min_commission;                  //最小手续费

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

                    std::string get_account_id()
                    { return std::string(account_id); }

                    std::string get_broker_id()
                    { return std::string(broker_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                //订单输入
                struct OrderInput
                {
                    uint64_t order_id;                       //订单ID

                    char instrument_id[INSTRUMENT_ID_LEN];   //合约代码
                    char exchange_id[EXCHANGE_ID_LEN];       //交易所代码
                    char account_id[ACCOUNT_ID_LEN];         //账号ID

                    InstrumentType instrument_type;          //合约类型

                    double limit_price;                      //价格
                    double frozen_price;                     //冻结价格

                    int64_t volume;                          //数量

                    Side side;                               //买卖方向
                    Offset offset;                           //开平方向
                    PriceType price_type;                    //价格类型
                    VolumeCondition volume_condition;        //成交量类型
                    TimeCondition time_condition;            //成交时间类型

                    uint64_t parent_id;                      //母订单ID

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

                    std::string get_account_id()
                    { return std::string(account_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const OrderInput &input)
                {
                    j["order_id"] = input.order_id;
                    j["instrument_id"] = std::string(input.instrument_id);
                    j["exchange_id"] = std::string(input.exchange_id);
                    j["account_id"] = std::string(input.account_id);
                    j["instrument_type"] = input.instrument_type;
                    j["volume"] = input.volume;
                    j["limit_price"] = FORMAT_DOUBLE(input.limit_price);
                    j["frozen_price"] = FORMAT_DOUBLE(input.frozen_price);
                    j["side"] = input.side;
                    j["offset"] = input.offset;
                    j["price_type"] = input.price_type;
                    j["volume_condition"] = input.volume_condition;
                    j["time_condition"] = input.time_condition;
                    j["parent_id"] = input.parent_id;
                }

                inline void from_json(const nlohmann::json &j, OrderInput &input)
                {
                    input.order_id = j["order_id"].get<uint64_t>();
                    strncpy(input.instrument_id, j["instrument_id"].get<std::string>().c_str(), INSTRUMENT_ID_LEN);
                    strncpy(input.exchange_id, j["exchange_id"].get<std::string>().c_str(), EXCHANGE_ID_LEN);
                    strncpy(input.account_id, j["account_id"].get<std::string>().c_str(), ACCOUNT_ID_LEN);
                    input.instrument_type = j["instrument_type"];
                    input.limit_price = j["limit_price"].get<double>();
                    input.frozen_price = j["frozen_price"].get<double>();
                    input.volume = j["volume"].get<int64_t>();
                    input.side = j["side"];
                    input.offset = j["offset"];
                    input.price_type = j["price_type"];
                    input.volume_condition = j["volume_condition"];
                    input.time_condition = j["time_condition"];
                    input.parent_id = j["parent_id"].get<uint64_t>();
                }

                //订单输入反馈(手动下单)
                struct OrderInputRsp
                {
                    uint64_t order_id;                       //订单ID

                    int error_id;                            //错误ID
                    char error_msg[ERROR_MSG_LEN];           //错误信息
#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const OrderInputRsp &input_rsp)
                {
                    j["order_id"] = input_rsp.order_id;
                    j["error_id"] = input_rsp.error_id;
                    j["error_msg"] = input_rsp.error_msg;
                }

                //订单操作
                struct OrderAction
                {
                    uint64_t order_id;                       //订单ID
                    uint64_t order_action_id;                //订单操作ID

                    OrderActionFlag action_flag;             //订单操作类型

                    double price;                            //价格
                    int64_t volume;                          //数量
#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const OrderAction &action)
                {
                    j["order_id"] = action.order_id;
                    j["order_action_id"] = action.order_action_id;
                    j["action_flag"] = action.action_flag;
                    j["price"] = FORMAT_DOUBLE(action.price);
                    j["volume"] = action.volume;
                }

                inline void from_json(const nlohmann::json &j, OrderAction &action)
                {
                    action.order_id = j["order_id"].get<uint64_t>();
                    action.order_action_id = j["order_action_id"].get<uint64_t>();
                    action.action_flag = j["action_flag"];
                    action.price = j["price"].get<double>();
                    action.volume = j["volume"].get<int64_t>();
                }

                //订单操作反馈(手动下单)
                struct OrderActionRsp
                {
                    uint64_t order_id;                       //订单ID
                    uint64_t order_action_id;                //订单操作ID

                    int error_id;                            //错误ID
                    char error_msg[ERROR_MSG_LEN];           //错误信息
#ifndef _WIN32
                } __attribute__((packed));
#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const OrderActionRsp &action_rsp)
                {
                    j["order_id"] = action_rsp.order_id;
                    j["order_action_id"] = action_rsp.order_action_id;
                    j["error_id"] = action_rsp.error_id;
                    j["error_msg"] = action_rsp.error_msg;
                }

                //订单消息
                struct Order
                {
                    uint64_t parent_id;                      //母订单ID
                    uint64_t order_id;                       //订单ID
                    uint64_t external_id;

                    int64_t insert_time;                     //订单写入时间
                    int64_t update_time;                     //订单更新时间

                    char trading_day[DATE_LEN];              //交易日

                    char instrument_id[INSTRUMENT_ID_LEN];   //合约ID
                    char exchange_id[EXCHANGE_ID_LEN];       //交易所ID
                    char account_id[ACCOUNT_ID_LEN];         //账号ID
                    char client_id[CLIENT_ID_LEN];           //Client ID

                    InstrumentType instrument_type;          //合约类型

                    double limit_price;                      //价格
                    double frozen_price;                     //冻结价格，市价单冻结价格为0

                    int64_t volume;                          //数量
                    int64_t volume_traded;                   //成交数量
                    int64_t volume_left;                     //剩余数量

                    double tax;                              //税
                    double commission;                       //手续费

                    OrderStatus status;                      //订单状态

                    int error_id;                            //错误ID
                    char error_msg[ERROR_MSG_LEN];           //错误信息

                    Side side;                               //买卖方向
                    Offset offset;                           //开平方向
                    PriceType price_type;                    //价格类型
                    VolumeCondition volume_condition;        //成交量类型
                    TimeCondition time_condition;            //成交时间类型

                    std::string get_trading_day()
                    { return std::string(trading_day); }

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

                    std::string get_account_id()
                    { return std::string(account_id); }

                    std::string get_client_id()
                    { return std::string(client_id); }

                    std::string get_error_msg()
                    { return std::string(error_msg); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Order &order)
                {
                    j["order_id"] = std::to_string(order.order_id);
                    j["insert_time"] = order.insert_time;
                    j["update_time"] = order.update_time;
                    j["trading_day"] = std::string(order.trading_day);

                    j["instrument_id"] = std::string(order.instrument_id);
                    j["exchange_id"] = std::string(order.exchange_id);

                    j["account_id"] = std::string(order.account_id);
                    j["client_id"] = std::string(order.client_id);

                    j["instrument_type"] = order.instrument_type;

                    j["limit_price"] = FORMAT_DOUBLE(order.limit_price);
                    j["frozen_price"] = FORMAT_DOUBLE(order.frozen_price);

                    j["volume"] = order.volume;
                    j["volume_traded"] = order.volume_traded;
                    j["volume_left"] = order.volume_left;

                    j["tax"] = FORMAT_DOUBLE(order.tax);
                    j["commission"] = FORMAT_DOUBLE(order.commission);

                    j["error_id"] = order.error_id;
                    j["error_msg"] = std::string(order.error_msg);

                    j["status"] = order.status;

                    j["parent_id"] = order.parent_id;

                    j["side"] = order.side;
                    j["offset"] = order.offset;
                    j["price_type"] = order.price_type;
                    j["volume_condition"] = order.volume_condition;
                    j["time_condition"] = order.time_condition;

                }

                //成交信息
                struct Trade
                {
                    uint64_t trade_id;                             //成交ID

                    uint64_t order_id;                       //订单ID
                    uint64_t parent_order_id;                //母订单ID

                    int64_t trade_time;                     //成交时间

                    char instrument_id[INSTRUMENT_ID_LEN];   //合约ID
                    char exchange_id[EXCHANGE_ID_LEN];       //交易所ID
                    char account_id[ACCOUNT_ID_LEN];         //账号ID
                    char client_id[CLIENT_ID_LEN];           //Client ID

                    InstrumentType instrument_type;          //合约类型

                    Side side;                               //买卖方向
                    Offset offset;                           //开平方向

                    double price;                            //成交价格
                    int64_t volume;                          //成交量

                    double tax;                              //税
                    double commission;                       //手续费

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

                    std::string get_account_id()
                    { return std::string(account_id); }

                    std::string get_client_id()
                    { return std::string(client_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Trade &trade)
                {
                    j["trade_id"] = trade.trade_id;
                    j["order_id"] = std::to_string(trade.order_id);

                    j["trade_time"] = trade.trade_time;
                    j["instrument_id"] = std::string(trade.instrument_id);
                    j["exchange_id"] = std::string(trade.exchange_id);
                    j["account_id"] = std::string(trade.account_id);
                    j["client_id"] = std::string(trade.client_id);

                    j["instrument_type"] = trade.instrument_type;

                    j["side"] = trade.side;
                    j["offset"] = trade.offset;

                    j["price"] = FORMAT_DOUBLE(trade.price);
                    j["volume"] = trade.volume;

                    j["tax"] = FORMAT_DOUBLE(trade.tax);
                    j["commission"] = FORMAT_DOUBLE(trade.commission);

                }

                //账户信息
                struct AccountInfo
                {
                    int64_t update_time;               //更新时间
                    char trading_day[DATE_LEN];        //交易日

                    char account_id[ACCOUNT_ID_LEN];   //账号ID
                    char broker_id[BROKER_ID_LEN];     //Broker ID
                    char source_id[SOURCE_ID_LEN];     //柜台ID

                    double initial_equity;             //期初权益
                    double static_equity;              //静态权益
                    double dynamic_equity;             //动态权益
                    double accumulated_pnl;            //累计收益
                    double accumulated_pnl_ratio;      //累计收益率
                    double intraday_pnl;               //日内收益
                    double intraday_pnl_ratio;         //日内收益率
                    double avail;                      //可用资金
                    double market_value;               //市值(股票)
                    double margin;                     //保证金(期货)
                    double accumulated_fee;            //累计手续费
                    double intraday_fee;               //当日手续费

                    double frozen_cash;                //冻结资金(股票: 买入挂单资金; 期货: 冻结保证金+冻结手续费)
                    double frozen_margin;              //冻结保证金(期货)
                    double frozen_fee;                 //冻结手续费(期货)

                    double position_pnl;               //持仓盈亏(期货)
                    double close_pnl;                  //平仓盈亏(期货)

                    std::string get_trading_day()
                    { return std::string(trading_day); }

                    std::string get_account_id()
                    { return std::string(account_id); }

                    std::string get_broker_id()
                    { return std::string(broker_id); }

                    std::string get_source_id()
                    { return std::string(source_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const AccountInfo &account)
                {
                    j["update_time"] = account.update_time;
                    j["trading_day"] = std::string(account.trading_day);
                    j["account_id"] = std::string(account.account_id);
                    j["broker_id"] = std::string(account.broker_id);
                    j["source_id"] = std::string(account.source_id);
                    j["initial_equity"] = FORMAT_DOUBLE(account.initial_equity);
                    j["static_equity"] = FORMAT_DOUBLE(account.static_equity);
                    j["dynamic_equity"] = FORMAT_DOUBLE(account.dynamic_equity);
                    j["accumulated_pnl"] = FORMAT_DOUBLE(account.accumulated_pnl);
                    j["accumulated_pnl_ratio"] = FORMAT_DOUBLE(account.accumulated_pnl_ratio);
                    j["intraday_pnl"] = FORMAT_DOUBLE(account.intraday_pnl);
                    j["intraday_pnl_ratio"] = FORMAT_DOUBLE(account.intraday_pnl_ratio);
                    j["avail"] = FORMAT_DOUBLE(account.avail);
                    j["market_value"] = FORMAT_DOUBLE(account.market_value);
                    j["margin"] = FORMAT_DOUBLE(account.margin);
                    j["accumulated_fee"] = FORMAT_DOUBLE(account.accumulated_fee);
                    j["intraday_fee"] = FORMAT_DOUBLE(account.intraday_fee);
                    j["frozen_cash"] = FORMAT_DOUBLE(account.frozen_cash);
                    j["frozen_margin"] = FORMAT_DOUBLE(account.frozen_margin);
                    j["frozen_fee"] = FORMAT_DOUBLE(account.frozen_fee);
                    j["position_pnl"] = FORMAT_DOUBLE(account.position_pnl);
                    j["close_pnl"] = FORMAT_DOUBLE(account.close_pnl);
                }

                //持仓信息
                struct Position
                {
                    int64_t update_time;                     //更新时间
                    char trading_day[DATE_LEN];              //交易日

                    char instrument_id[INSTRUMENT_ID_LEN];   //合约ID
                    InstrumentType instrument_type;          //合约类型
                    char exchange_id[EXCHANGE_ID_LEN];       //交易所ID

                    char account_id[ACCOUNT_ID_LEN];         //账号ID
                    char client_id[CLIENT_ID_LEN];           //Client ID

                    Direction direction;                     //持仓方向

                    int64_t volume;                          //数量
                    int64_t yesterday_volume;                //昨仓数量
                    int64_t frozen_total;                    //冻结数量
                    int64_t frozen_yesterday;                //冻结昨仓

                    double last_price;                       //最新价
                    double open_price;                       //开仓价(期货明细为开仓价; 期货统计为开仓均价，加权平均值; 股票无意义，为0)

                    // 成本价
                    // 股票: (历史买入总资金 - 历史卖出总资金) / 当前仓位, 如果当前仓位是0则为0
                    // 期货: ((昨结算 * 昨仓量 + SUM(今仓开仓价 * 量)) * 合约乘数 + SUM(今手续费)) / 合约乘数
                    double cost_price;

                    double close_price;                      //收盘价(股票和债券)
                    double pre_close_price;                  //昨收价(股票和债券)
                    double settlement_price;                 //结算价(期货)
                    double pre_settlement_price;             //昨结算(期货)

                    double margin;                           //保证金(期货)
                    double position_pnl;                     //持仓盈亏(期货)
                    double close_pnl;                        //平仓盈亏(期货)

                    double realized_pnl;                     //已实现盈亏
                    double unrealized_pnl;                   //未实现盈亏

                    char open_date[DATE_LEN];                //开仓日(YYYYMMDD,仅期货明细和债券)
                    char expire_date[DATE_LEN];              //到期日(YYYYMMDD,仅债券)

                    std::string get_trading_day()
                    { return std::string(trading_day); }

                    std::string get_instrument_id()
                    { return std::string(instrument_id); }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

                    std::string get_account_id()
                    { return std::string(account_id); }

                    std::string get_client_id()
                    { return std::string(client_id); }

                    std::string get_open_date()
                    { return std::string(open_date); }

                    std::string get_expire_date()
                    { return std::string(expire_date); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Position &position)
                {
                    j["update_time"] = position.update_time;
                    j["instrument_id"] = std::string(position.instrument_id);
                    j["instrument_type"] = position.instrument_type;
                    j["exchange_id"] = std::string(position.exchange_id);
                    j["account_id"] = std::string(position.account_id);
                    j["client_id"] = std::string(position.client_id);
                    j["direction"] = position.direction;
                    j["volume"] = position.volume;
                    j["yesterday_volume"] = position.yesterday_volume;
                    j["frozen_total"] = position.frozen_total;
                    j["frozen_yesterday"] = position.frozen_yesterday;
                    j["last_price"] = FORMAT_DOUBLE(position.last_price);
                    j["open_price"] = FORMAT_DOUBLE(position.open_price);
                    j["cost_price"] = FORMAT_DOUBLE(position.cost_price);
                    j["close_price"] = FORMAT_DOUBLE(position.close_price);
                    j["pre_close_price"] = FORMAT_DOUBLE(position.pre_close_price);
                    j["settlement_price"] = FORMAT_DOUBLE(position.settlement_price);
                    j["pre_settlement_price"] = FORMAT_DOUBLE(position.pre_settlement_price);
                    j["margin"] = FORMAT_DOUBLE(position.margin);
                    j["position_pnl"] = FORMAT_DOUBLE(position.position_pnl);
                    j["close_pnl"] = FORMAT_DOUBLE(position.close_pnl);
                    j["realized_pnl"] = FORMAT_DOUBLE(position.realized_pnl);
                    j["unrealized_pnl"] = FORMAT_DOUBLE(position.unrealized_pnl);
                    j["open_date"] = std::string(position.open_date);
                    j["expire_date"] = std::string(position.expire_date);
                }

                //投资组合信息
                struct PortfolioInfo
                {
                    char trading_day[DATE_LEN];    //交易日
                    int64_t update_time;           //数据更新时间

                    char client_id[CLIENT_ID_LEN]; //Client ID

                    double initial_equity;         //期初权益
                    double static_equity;          //静态权益
                    double dynamic_equity;         //动态权益
                    double accumulated_pnl;        //累计收益
                    double accumulated_pnl_ratio;  //累计收益率
                    double intraday_pnl;           //日内收益
                    double intraday_pnl_ratio;     //日内收益率

                    std::string get_trading_day()
                    { return std::string(trading_day); }

                    std::string get_client_id()
                    { return std::string(client_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const PortfolioInfo &pnl)
                {
                    j["trading_day"] = std::string(pnl.trading_day);
                    j["update_time"] = pnl.update_time;
                    j["initial_equity"] = FORMAT_DOUBLE(pnl.initial_equity);
                    j["static_equity"] = FORMAT_DOUBLE(pnl.static_equity);
                    j["dynamic_equity"] = FORMAT_DOUBLE(pnl.dynamic_equity);
                    j["accumulated_pnl"] = FORMAT_DOUBLE(pnl.accumulated_pnl);
                    j["accumulated_pnl_ratio"] = FORMAT_DOUBLE(pnl.accumulated_pnl_ratio);
                    j["intraday_pnl"] = FORMAT_DOUBLE(pnl.intraday_pnl);
                    j["intraday_pnl_ratio"] = FORMAT_DOUBLE(pnl.intraday_pnl_ratio);
                }

                typedef AccountInfo SubPortfolioInfo; // 策略中的单个账户数据

                template<typename T>
                inline std::string to_string(const T &ori)
                {
                    nlohmann::json j;
                    to_json(j, ori);
                    return j.dump();
                }
            }


            namespace nanomsg
            {

                //持仓冻结信息
                struct PositionFrozen
                {
                    char instrument_id[INSTRUMENT_ID_LEN];   //合约代码
                    InstrumentType instrument_type;          //合约类型
                    char exchange_id[EXCHANGE_ID_LEN];       //交易所代码
                    Direction direction;                     //持仓方向
                    int64_t volume;                          //冻结的持仓
                    int64_t yesterday_volume;                //冻结的昨仓
                };

                //持仓修改信息
                typedef PositionFrozen PositionDiff;

                //资产冻结请求
                struct AssetsFrozen
                {
                    char account_id[ACCOUNT_ID_LEN];                //账号ID
                    char client_id[CLIENT_ID_LEN];                  //Client ID
                    double frozen_cash;                             //冻结资金
                    std::vector<PositionFrozen> frozen_positions;   //冻结持仓
                };

                //持仓修改请求
                struct PositionModify
                {
                    char account_id[ACCOUNT_ID_LEN];                //账户ID
                    char client_id[CLIENT_ID_LEN];                  //Client ID
                    std::vector<PositionDiff> position_diffs;       //修改持仓
                };

                //算法订单输入
                struct AlgoOrderInput
                {
                    uint64_t order_id;                              //订单ID
                    std::string client_id;                          //Client ID
                    std::string algo_type;                          //算法订单类型
                    std::string input;                              //订单输入信息
                };

                inline void from_json(const nlohmann::json &j, AlgoOrderInput &input)
                {
                    input.order_id = j["order_id"];
                    input.client_id = j["client_id"];
                    input.algo_type = j["algo_type"];
                    input.input = j["input"];
                }

                inline void to_json(nlohmann::json &j, const AlgoOrderInput &input)
                {
                    j["order_id"] = input.order_id;
                    j["client_id"] = input.client_id;
                    j["algo_type"] = input.algo_type;
                    j["input"] = input.input;
                }

                //算法订单操作
                struct AlgoOrderAction
                {
                    uint64_t order_id;                              //订单ID
                    uint64_t order_action_id;                       //订单操作ID
                    std::string action;                             //订单操作信息
                };

                inline void from_json(const nlohmann::json &j, AlgoOrderAction &action)
                {
                    action.order_id = j["order_id"];
                    action.order_action_id = j["order_action_id"];
                    action.action = j["action"];
                }

                inline void to_json(nlohmann::json &j, const AlgoOrderAction &action)
                {
                    j["order_id"] = action.order_id;
                    j["order_action_id"] = action.order_action_id;
                    j["action"] = action.action;
                }

                struct AlgoOrderStatus
                {
                    uint64_t order_id;
                    std::string algo_type;
                    std::string status;
                };

                inline void from_json(const nlohmann::json &j, AlgoOrderStatus &status)
                {
                    status.order_id = j["order_id"];
                    status.algo_type = j["algo_type"];
                    status.status = j["status"];
                }

                inline void to_json(nlohmann::json &j, const AlgoOrderStatus &status)
                {
                    j["order_id"] = status.order_id;
                    j["algo_type"] = status.algo_type;
                    j["status"] = status.status;
                }

                //交易Session
                struct TradingSession
                {
                    std::string exchange_id;           //交易所ID
                    std::string trading_day;           //交易日
                    int64_t start_nano;                //开始时间纳秒时间戳
                    int64_t end_nano;                  //结束时间纳秒时间戳
                };

                template<typename T>
                std::string to_string(const T &ori)
                {
                    nlohmann::json j;
                    to_json(j, ori);
                    return j.dump();
                }
            }
        }
    }
}

#endif //WINGCHUN_EVENT_H
