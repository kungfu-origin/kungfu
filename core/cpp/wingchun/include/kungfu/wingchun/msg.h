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
#include <kungfu/yijinjing/journal/journal.h>

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

                    Bar = 110,

                    OrderInput = 201,
                    OrderAction = 202,
                    Order = 203,
                    Trade = 204,
                    Position = 205,
                    Asset = 206,
                    AssetSnapshot = 207,
                    PositionDetail = 208,
                    Instrument = 209,

                    Subscribe = 302,
                    NewOrderSingle = 353,
                    CancelOrder = 354,
                    CancelAllOrder = 355,

                    BrokerStateRefresh = 400,
                    BrokerState = 401,
                    QryAsset = 402,
                    PublishAllAssetInfo = 403,
                    RemoveStrategy = 404,
                    
                    PositionEnd = 800,
                    PositionDetailEnd = 801,
                    InstrumentEnd = 802,
                };
            }

            namespace data
            {
                enum class BrokerState : int
                {
                    Unknown = 0,
                    Idle = 1,
                    DisConnected = 2,
                    Connected = 3,
                    LoggedIn = 4,
                    LoggedInFailed = 5,
                    Ready = 100
                };

                struct Instrument
                {
                    char instrument_id[INSTRUMENT_ID_LEN];     //合约ID
                    char exchange_id[EXCHANGE_ID_LEN];         //交易所ID
                    InstrumentType instrument_type;            //合约类型

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

                    const std::string get_instrument_id() const
                    { return std::string(instrument_id); }

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    const std::string get_product_id() const
                    { return std::string(product_id); }

                    const std::string get_open_date() const
                    { return std::string(open_date); }

                    const std::string get_create_date() const
                    { return std::string(create_date); }

                    const std::string get_expire_date() const
                    { return std::string(expire_date); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Instrument &instrument)
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

                struct FutureInstrument : public Instrument
                {};

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

                    std::string get_source_id() const
                    { return std::string(source_id); }

                    void set_source_id(const std::string & source_id_)
                    {strncpy(source_id, source_id_.c_str(), SOURCE_ID_LEN);}

                    std::string get_trading_day() const
                    { return std::string(trading_day); }

                    void set_trading_day(const std::string & trading_day_)
                    {strncpy(trading_day, trading_day_.c_str(), DATE_LEN);}

                    std::string get_instrument_id() const
                    { return std::string(instrument_id); }

                    void set_instrument_id(const std::string & instrument_id_)
                    { strncpy(instrument_id, instrument_id_.c_str(), INSTRUMENT_ID_LEN);}

                    std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string & exchange_id_)
                    {strncpy(exchange_id, exchange_id_.c_str(), EXCHANGE_ID_LEN);}

                    std::vector<double> get_bid_price() const
                    { return std::vector<double>(bid_price, bid_price + 10); }

                    void set_bid_price(const std::vector<double> &bp)
                    { memcpy(bid_price, (const void*) bp.data(), sizeof(double) * std::min(10, int(bp.size())));}

                    std::vector<double> get_ask_price() const
                    { return std::vector<double>(ask_price, ask_price + 10); }

                    void set_ask_price(const std::vector<double> &ap)
                    { memcpy(ask_price, (const void*) ap.data(), sizeof(double) * std::min(10, int(ap.size())));}

                    std::vector<int64_t> get_bid_volume() const
                    { return std::vector<int64_t>(bid_volume, bid_volume + 10); }

                    void set_bid_volume(const std::vector<int64_t> &bv)
                    { memcpy(bid_volume, (const void*) bv.data(), sizeof(int64_t) * std::min(10, int(bv.size())));}

                    std::vector<int64_t> get_ask_volume() const
                    { return std::vector<int64_t>(ask_volume, ask_volume + 10); }

                    void set_ask_volume(const std::vector<int64_t> &av)
                    { memcpy(ask_volume, (const void*) av.data(), sizeof(int64_t) * std::min(10, int(av.size())));}

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

                    j["pre_close_price"] = quote.pre_close_price;
                    j["pre_settlement_price"] = quote.pre_settlement_price;

                    j["last_price"] = quote.last_price;
                    j["volume"] = quote.volume;
                    j["turnover"] = quote.turnover;

                    j["pre_open_interest"] = quote.pre_open_interest;
                    j["open_interest"] = quote.open_interest;

                    j["open_price"] = quote.open_price;
                    j["high_price"] = quote.high_price;
                    j["low_price"] = quote.low_price;

                    j["upper_limit_price"] = quote.upper_limit_price;
                    j["lower_limit_price"] = quote.lower_limit_price;

                    j["close_price"] = quote.close_price;
                    j["settlement_price"] = quote.settlement_price;

                    j["bid_price"] = quote.get_bid_price();
                    j["ask_price"] = quote.get_ask_price();

                    j["bid_volume"] = quote.get_bid_volume();
                    j["ask_volume"] = quote.get_ask_volume();
                }

                inline void from_json(const nlohmann::json &j, Quote &quote)
                {
                    quote.set_trading_day(j["trading_day"].get<std::string>());
                    quote.data_time = j["data_time"];
                    quote.set_instrument_id(j["instrument_id"].get<std::string>());
                    quote.set_exchange_id(j["exchange_id"].get<std::string>());
                    quote.instrument_type = j["instrument_type"];
                    quote.pre_close_price = j["pre_close_price"];
                    quote.pre_settlement_price = j["pre_settlement_price"];
                    quote.last_price = j["last_price"];
                    quote.volume = j["volume"];
                    quote.turnover = j["turnover"];
                    quote.pre_open_interest = j["pre_open_interest"];
                    quote.open_interest = j["open_interest"];

                    quote.open_price = j["open_price"];
                    quote.high_price = j["high_price"];
                    quote.low_price = j["low_price"];

                    quote.upper_limit_price = j["upper_limit_price"];
                    quote.lower_limit_price = j["lower_limit_price"];

                    quote.close_price = j["close_price"];
                    quote.settlement_price = j["settlement_price"];

                    quote.set_bid_price(j["bid_price"].get<std::vector<double>>());
                    quote.set_ask_price(j["ask_price"].get<std::vector<double>>());
                    quote.set_bid_volume(j["bid_volume"].get<std::vector<int64_t>>());
                    quote.set_ask_volume(j["ask_volume"].get<std::vector<int64_t>>());
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
                    j["price"] = entrust.price;
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
                    j["price"] = transaction.price;
                    j["volume"] = transaction.volume;
                    j["bid_no"] = transaction.bid_no;
                    j["ask_no"] = transaction.ask_no;
                    j["exec_type"] = transaction.exec_type;
                    j["bs_flag"] = transaction.bs_flag;
                    j["main_seq"] = transaction.main_seq;
                    j["seq"] = transaction.seq;
                }

                struct Bar
                {
                    char trading_day[DATE_LEN];            //交易日 
                    char instrument_id[INSTRUMENT_ID_LEN]; //合约代码
                    double pre_close_price;                //昨收价
                    int64_t start_time;                    //开始时间
                    int64_t end_time;                      //结束时间
                    double open;                           //开
                    double close;                          //收
                    double low;                            //低
                    double high;                           //高
                    int volume;                            //区间交易量
                    int start_volume;                       //初始总交易量
                    int count;                             //区间有效tick数
                    int64_t next_time;                     //下次开始时间
                };



                inline void to_json(nlohmann::json &j, const Bar &bar)
                {
                    j["trading_day"] = std::string(bar.trading_day);
                    j["instrument_id"] = std::string(bar.instrument_id);
                    j["pre_close_price"] = bar.pre_close_price;
                    j["start_time"] = bar.start_time;
                    j["end_time"] = bar.end_time;
                    j["open"] = bar.open;
                    j["close"] = bar.close;
                    j["low"] = bar.low;
                    j["high"] = bar.high;
                    j["volume"] = bar.volume;
                    j["start_volume"] = bar.start_volume;
                    j["count"] = bar.count;
                    j["next_time"] = bar.next_time;
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

                    void set_instrument_id(const std::string &inst)
                    {
                        strcpy(instrument_id, inst.c_str());
                    }

                    std::string get_exchange_id()
                    { return std::string(exchange_id); }

                    void set_exchange_id(const std::string &exch)
                    {
                        strcpy(exchange_id, exch.c_str());
                    }

                    std::string get_account_id()
                    { return std::string(account_id); }

                    void set_account_id(const std::string &account)
                    {
                        strcpy(account_id, account.c_str());
                    }
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
                    j["limit_price"] = input.limit_price;
                    j["frozen_price"] =  input.frozen_price;
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
                    j["price"] = action.price;
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

                    j["limit_price"] = order.limit_price;
                    j["frozen_price"] = order.frozen_price;

                    j["volume"] = order.volume;
                    j["volume_traded"] = order.volume_traded;
                    j["volume_left"] = order.volume_left;

                    j["tax"] = order.tax;
                    j["commission"] = order.commission;

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

                inline void from_json(const nlohmann::json &j, Order &order)
                {
                    order.order_id = j["order_id"];
                    order.insert_time = j["insert_time"];
                    order.update_time = j["update_time"];
                    strncpy(order.trading_day, j["trading_day"].get<std::string>().c_str(), DATE_LEN);
                    strncpy(order.instrument_id, j["instrument_id"].get<std::string>().c_str(), INSTRUMENT_ID_LEN);
                    strncpy(order.exchange_id, j["exchange_id"].get<std::string>().c_str(), EXCHANGE_ID_LEN);

                    strncpy(order.account_id, j["account_id"].get<std::string>().c_str(), ACCOUNT_ID_LEN);
                    strncpy(order.client_id, j["client_id"].get<std::string>().c_str(), CLIENT_ID_LEN);

                    order.instrument_type = j["instrument_type"];

                    order.limit_price = j["limit_price"].get<double>();
                    order.frozen_price = j["frozen_price"].get<double>();

                    order.volume = j["volume"];
                    order.volume_traded = j["volume_traded"];
                    order.volume_left = j["volume_left"];

                    order.tax = j["tax"].get<double>();
                    order.commission = j["commission"].get<double>();

                    order.error_id = j["error_id"];
                    strncpy(order.error_msg, j["error_msg"].get<std::string>().c_str(), ERROR_MSG_LEN);

                    order.status = j["status"];

                    order.parent_id = j["parent_id"];

                    order.side = j["side"];
                    order.offset = j["offset"];
                    order.price_type = j["price_type"];
                    order.volume_condition = j["volume_condition"];
                    order.time_condition = j["time_condition"];
                }

                inline void order_from_input(const msg::data::OrderInput &input, msg::data::Order &order)
                {
                    order.parent_id = input.parent_id;
                    order.order_id = input.order_id;
                    strcpy(order.instrument_id, input.instrument_id);
                    strcpy(order.exchange_id, input.exchange_id);
                    strcpy(order.account_id, input.account_id);

                    order.limit_price = input.limit_price;
                    order.frozen_price = input.frozen_price;

                    order.volume = input.volume;
                    order.volume_traded = 0;
                    order.volume_left = input.volume;
                    order.status = OrderStatus::Submitted;

                    order.side = input.side;
                    order.offset = input.offset;

                    order.price_type = input.price_type;
                    order.volume_condition = input.volume_condition;
                    order.time_condition = input.time_condition;

                }


                //成交信息
                struct Trade
                {
                    uint64_t trade_id;                             //成交ID

                    uint64_t order_id;                       //订单ID
                    uint64_t parent_order_id;                //母订单ID

                    int64_t trade_time;                     //成交时间
                    char trading_day[DATE_LEN];              //交易日

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

                    const std::string get_instrument_id() const
                    { return std::string(instrument_id); }

                    const std::string get_exchange_id() const
                    { return std::string(exchange_id); }

                    const std::string get_account_id() const
                    { return std::string(account_id); }

                    const std::string get_client_id() const
                    { return std::string(client_id); }

                    const std::string get_trading_day() const
                    {
                        return std::string(trading_day);
                    }
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
                    j["trading_day"] = std::string(trade.trading_day);

                    j["instrument_id"] = std::string(trade.instrument_id);
                    j["exchange_id"] = std::string(trade.exchange_id);
                    j["account_id"] = std::string(trade.account_id);
                    j["client_id"] = std::string(trade.client_id);

                    j["instrument_type"] = trade.instrument_type;

                    j["side"] = trade.side;
                    j["offset"] = trade.offset;

                    j["price"] = trade.price;
                    j["volume"] = trade.volume;

                    j["tax"] = trade.tax;
                    j["commission"] = trade.commission;

                }

                inline void from_json(const nlohmann::json &j, Trade &trade)
                {
                    trade.trade_id = j["trade_id"];
                    
                    trade.order_id = j["order_id"].get<uint64_t>();

                    trade.trade_time = j["trade_time"];
                    strncpy(trade.instrument_id, j["instrument_id"].get<std::string>().c_str(), INSTRUMENT_ID_LEN);
                    strncpy(trade.exchange_id, j["exchange_id"].get<std::string>().c_str(), EXCHANGE_ID_LEN);
                    strncpy(trade.account_id, j["account_id"].get<std::string>().c_str(), ACCOUNT_ID_LEN);
                    strncpy(trade.client_id, j["client_id"].get<std::string>().c_str(), CLIENT_ID_LEN);

                    trade.instrument_type = j["instrument_type"];

                    trade.side = j["side"];
                    trade.offset = j["offset"];

                    trade.price = j["price"].get<double>();
                    trade.volume = j["volume"];

                    trade.tax = j["tax"].get<double>();
                    trade.commission = j["commission"].get<double>();
                }

                //账户信息
                struct Asset
                {
                    int64_t update_time;               //更新时间
                    char trading_day[DATE_LEN];        //交易日

                    char account_id[ACCOUNT_ID_LEN];   //账号ID
                    char broker_id[BROKER_ID_LEN];     //Broker ID
                    char source_id[SOURCE_ID_LEN];     //柜台ID

                    char client_id[CLIENT_ID_LEN];     //client ID

                    double initial_equity;             //期初权益
                    double static_equity;              //静态权益
                    double dynamic_equity;             //动态权益

                    double realized_pnl;            //累计收益
                    double unrealized_pnl;

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

                    std::string get_client_id()
                    { return std::string(client_id); }

                    std::string get_source_id()
                    { return std::string(source_id); }

#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Asset &asset_info)
                {
                    j["update_time"] = asset_info.update_time;
                    j["trading_day"] = std::string(asset_info.trading_day);
                    j["account_id"] = std::string(asset_info.account_id);
                    j["broker_id"] = std::string(asset_info.broker_id);
                    j["source_id"] = std::string(asset_info.source_id);
                    j["client_id"] = std::string(asset_info.client_id);
                    j["initial_equity"] = asset_info.initial_equity;
                    j["static_equity"] = asset_info.static_equity;
                    j["dynamic_equity"] = asset_info.dynamic_equity;
                    j["unrealized_pnl"] = asset_info.unrealized_pnl;
                    j["realized_pnl"] =  asset_info.realized_pnl;
                    j["avail"] = asset_info.avail;
                    j["market_value"] = asset_info.market_value;
                    j["margin"] = asset_info.margin;
                    j["accumulated_fee"] = asset_info.accumulated_fee;
                    j["intraday_fee"] = asset_info.intraday_fee;
                    j["frozen_cash"] = asset_info.frozen_cash;
                    j["frozen_margin"] = asset_info.frozen_margin;
                    j["frozen_fee"] = asset_info.frozen_fee;
                    j["position_pnl"] = asset_info.position_pnl;
                    j["close_pnl"] = asset_info.close_pnl;
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

                    double avg_open_price;                   //开仓均价
                    double position_cost_price;              //持仓成本价

                    double close_price;                      //收盘价(股票和债券)
                    double pre_close_price;                  //昨收价(股票和债券)

                    double settlement_price;                 //结算价(期货)
                    double pre_settlement_price;             //昨结算(期货)

                    double margin;                           //保证金(期货)
                    double position_pnl;                     //持仓盈亏(期货)
                    double close_pnl;                        //平仓盈亏(期货)

                    double realized_pnl;                     //已实现盈亏
                    double unrealized_pnl;                   //未实现盈亏

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
#ifndef _WIN32
                } __attribute__((packed));

#else
                };
#pragma pack(pop)
#endif

                inline void to_json(nlohmann::json &j, const Position &position)
                {
                    j["update_time"] = position.update_time;
                    j["trading_day"] = std::string(position.trading_day);
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
                    j["last_price"] = position.last_price;
                    j["avg_open_price"] = position.avg_open_price;
                    j["position_cost_price"] = position.position_cost_price;
                    j["close_price"] = position.close_price;
                    j["pre_close_price"] = position.pre_close_price;
                    j["settlement_price"] = position.settlement_price;
                    j["pre_settlement_price"] = position.pre_settlement_price;
                    j["margin"] = position.margin;
                    j["position_pnl"] = position.position_pnl;
                    j["close_pnl"] = position.close_pnl;
                    j["realized_pnl"] = position.realized_pnl;
                    j["unrealized_pnl"] = position.unrealized_pnl;
                }

            //持仓明细信息
            struct PositionDetail
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
                int64_t frozen_volume;                   //冻结数量

                double last_price;                       //最新价
                double open_price;                       //开仓价

                double settlement_price;                 //结算价(期货)
                double pre_settlement_price;             //昨结算(期货)

                char open_date[DATE_LEN];                //开仓日(YYYYMMDD,仅期货明细和债券)

                uint64_t trade_id;                       //成交ID
                int64_t trade_time;                      //成交时间

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

#ifndef _WIN32
            } __attribute__((packed));

#else
            };
#pragma pack(pop)
#endif

            inline void to_json(nlohmann::json &j, const PositionDetail &detail)
            {
                j["update_time"] = detail.update_time;
                j["trading_day"] = std::string(detail.trading_day);
                j["instrument_id"] = std::string(detail.instrument_id);
                j["instrument_type"] = detail.instrument_type;
                j["exchange_id"] = std::string(detail.exchange_id);
                j["account_id"] = std::string(detail.account_id);
                j["client_id"] = std::string(detail.client_id);
                j["direction"] = detail.direction;
                j["volume"] = detail.volume;
                j["frozen_volume"] = detail.frozen_volume;
                j["last_price"] = detail.last_price;
                j["open_price"] = detail.open_price;
                j["settlement_price"] = detail.settlement_price;
                j["pre_settlement_price"] = detail.pre_settlement_price;
                j["open_date"] = std::string(detail.open_date);

                j["trade_id"] = detail.trade_id;
                j["trade_time"] = detail.trade_time;

            }

                template<typename T>
                inline std::string to_string(const T &ori)
                {
                    nlohmann::json j;
                    to_json(j, ori);
                    return j.dump(-1, ' ', false, nlohmann::json::error_handler_t::ignore);;
                }

            class MsgWriter
            {
            public:
                MsgWriter(kungfu::yijinjing::journal::writer_ptr writer): writer_(writer) {};
                void write_data(int msg_type, const std::string &json_str)
                {
                    switch (msg_type)
                    {
                        case kungfu::wingchun::msg::type::Quote:
                        {
                            auto j = nlohmann::json::parse(json_str);
                            Quote &quote = writer_->open_data<Quote>(0, msg::type::Quote);
                            from_json(j, quote);
                            writer_->close_data();
                            break;
                        }
                        default:
                            throw wingchun_error("unrecognized msg_type");
                    }
                }
            private:
                kungfu::yijinjing::journal::writer_ptr writer_;
            };

            class Decoder
                {
                public:
                    virtual ~Decoder() {}
                    virtual std::string json_from_ptr(uintptr_t address) const = 0;
                };

                template<typename T>
                class DecoderT: public Decoder
                {
                public:
                    virtual std::string json_from_ptr(uintptr_t address) const
                    {
                        const T& data = *(reinterpret_cast<const T *>(address));
                        return to_string(data);
                    }
                };

                typedef std::shared_ptr<Decoder> decoder_ptr;

                template<typename T>
                decoder_ptr make_decoder()
                {
                    return std::shared_ptr<Decoder>(new DecoderT<T>());
                }

                typedef std::unordered_map<int, decoder_ptr> decoder_map;

                inline decoder_ptr select_decoder(int msg_type)
                {
                    static decoder_map d_map;
                    if (d_map.empty())
                    {
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::Quote), make_decoder<msg::data::Quote>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::Entrust), make_decoder<msg::data::Entrust>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::Transaction), make_decoder<msg::data::Transaction>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::OrderInput), make_decoder<msg::data::OrderInput>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::OrderAction), make_decoder<msg::data::OrderAction>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::Order), make_decoder<msg::data::Order>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::Trade), make_decoder<msg::data::Trade>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::Position), make_decoder<msg::data::Position>()));
                        d_map.insert(std::make_pair(static_cast<int>(msg::type::Asset), make_decoder<msg::data::Asset>()));
                    }
                    const auto& it = d_map.find(msg_type);
                    return it == d_map.end() ? nullptr : it->second;
                }
                
                inline std::string json_from_address(int msg_type, uintptr_t address)
                {
                    decoder_ptr decoder = select_decoder(msg_type);
                    return decoder == nullptr ? "{}" : decoder->json_from_ptr(address);
                }    
            }
        }
    }
}

#endif //WINGCHUN_EVENT_H
