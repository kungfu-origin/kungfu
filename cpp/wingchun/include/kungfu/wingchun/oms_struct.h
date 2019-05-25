//
// Created by qlu on 2019/1/18.
//

#ifndef PROJECT_OMS_STRUCT_H
#define PROJECT_OMS_STRUCT_H

#include <kungfu/wingchun/constant.h>
#include <ctime>
#include <vector>
#include <string>

namespace kungfu {namespace journal
{
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

        std::string get_instrument_id() {return std::string(instrument_id);}
        std::string get_exchange_id() {return std::string(exchange_id);}
        std::string get_account_id() {return std::string(account_id);}
        std::string get_broker_id() {return std::string(broker_id);}
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
        char client_id[CLIENT_ID_LEN];           //Client ID

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

        std::string get_instrument_id() {return std::string(instrument_id);}
        std::string get_exchange_id() {return std::string(exchange_id);}
        std::string get_account_id() {return std::string(account_id);}
        std::string get_client_id() {return std::string(client_id);}
#ifndef _WIN32
    } __attribute__((packed));
#else
    };
    #pragma pack(pop)
#endif

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

    //订单消息
    struct Order
    {
        int64_t rcv_time;                        //数据接收时间

        uint64_t order_id;                       //订单ID

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

        uint64_t parent_id;                      //母订单ID

        Side side;                               //买卖方向
        Offset offset;                           //开平方向
        PriceType price_type;                    //价格类型
        VolumeCondition volume_condition;        //成交量类型
        TimeCondition time_condition;            //成交时间类型

        std::string get_trading_day() {return std::string(trading_day);}
        std::string get_instrument_id() {return std::string(instrument_id);}
        std::string get_exchange_id() {return std::string(exchange_id);}
        std::string get_account_id() {return std::string(account_id);}
        std::string get_client_id() {return std::string(client_id);}
        std::string get_error_msg() {return std::string(error_msg);}
#ifndef _WIN32
    } __attribute__((packed));
#else
    };
    #pragma pack(pop)
#endif
    
    //成交信息
    struct Trade
    {
        int64_t rcv_time;                        //数据接收时间

        uint64_t id;                             //成交ID

        uint64_t order_id;                       //订单ID
        uint64_t parent_order_id;                //母订单ID

        int64_t  trade_time;                     //成交时间

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

        std::string get_instrument_id() {return std::string(instrument_id);}
        std::string get_exchange_id() {return std::string(exchange_id);}
        std::string get_account_id() {return std::string(account_id);}
        std::string get_client_id() {return std::string(client_id);}
#ifndef _WIN32
    } __attribute__((packed));
#else
    };
    #pragma pack(pop)
#endif

}}

namespace kungfu {namespace flying
{
    //账户信息
    struct AccountInfo
    {
        int64_t rcv_time;                  //数据接收时间
        int64_t update_time;               //更新时间
        char trading_day[DATE_LEN];        //交易日

        char account_id[ACCOUNT_ID_LEN];   //账号ID
        AccountType account_type;                  //账号类型
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

        std::string get_trading_day() {return std::string(trading_day);}
        std::string get_account_id() {return std::string(account_id);}
        std::string get_broker_id() {return std::string(broker_id);}
        std::string get_source_id() {return std::string(source_id);}
    };

    //持仓信息
    struct Position
    {
        int64_t rcv_time;                        //数据接收时间
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

        std::string get_trading_day() {return std::string(trading_day);}
        std::string get_instrument_id() {return std::string(instrument_id);}
        std::string get_exchange_id() {return std::string(exchange_id);}
        std::string get_account_id() {return std::string(account_id);}
        std::string get_client_id() {return std::string(client_id);}
        std::string get_open_date() {return std::string(open_date);}
        std::string get_expire_date() {return std::string(expire_date);}
    };

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

        std::string get_trading_day() {return std::string(trading_day);}
        std::string get_client_id() {return std::string(client_id);}
    };

    typedef AccountInfo SubPortfolioInfo; // 策略中的单个账户数据

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

    //算法订单操作
    struct AlgoOrderAction
    {
        uint64_t order_id;                              //订单ID
        uint64_t order_action_id;                       //订单操作ID
        std::string action;                             //订单操作信息
    };

    struct AlgoOrderStatus
    {
        uint64_t order_id;
        std::string algo_type;
        std::string status;
    };

    //交易Session
    struct TradingSession
    {
        std::string exchange_id;           //交易所ID
        std::string trading_day;           //交易日
        int64_t start_nano;                //开始时间纳秒时间戳
        int64_t end_nano;                  //结束时间纳秒时间戳
    };
}}

#endif //PROJECT_OMS_STRUCT_H
