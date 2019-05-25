//
// Created by qlu on 2019/1/16.
//

#ifndef WC_2_STRATEGY_H
#define WC_2_STRATEGY_H


#include <string>
#include <vector>
#include <memory>
#include <functional>

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>

namespace kungfu
{
    class Strategy
    {
    public:
        explicit Strategy(const std::string& name);
        virtual ~Strategy();

        const std::string& get_name();

        // 开始运行
        void run();

        // 结束运行
        void stop();

        //添加策略使用的行情柜台
        //@param source_id   柜台ID
        //@return            成功或者失败
        bool add_md(const std::string& source_id);

        //添加策略使用的交易账户
        //@param source_id   柜台ID
        //@param account_id  账户ID
        //@param cash_limit  可用资金限制
        //@return            成功或者失败
        bool add_account(const std::string& source_id, const std::string& account_id, double cash_limit);

        //注册算法订单服务
        //@return            成功或者失败
        bool register_algo_service();

        //获取当前纳秒时间
        //@return            当前纳秒时间
        int64_t get_nano() const;

        //获取接收到的最新一条行情数据
        //@param instrument_id 合约ID
        //@param exchange_id   交易所ID
        //@param account_id    账户ID
        //@return              行情数据指针(nullptr 表示暂未收到行情数据)
        const journal::Quote* const get_last_md(const std::string& instrument_id, const std::string& exchange_id) const;

        //获取持仓信息
        //@param instrument_id 合约ID
        //@param exchange_id   交易所ID
        //@param direction     持仓方向
        //@param account_id    账户ID("" 表示获取策略在所有账户下持仓汇总信息)
        //@return              持仓数据(无持仓时volume是0)
        flying::Position get_position(const std::string& instrument_id, const std::string& exchange_id, Direction direction = DirectionLong, const std::string& account_id = "") const;

        flying::PortfolioInfo get_portfolio_info() const;

        flying::SubPortfolioInfo get_sub_portfolio_info(const std::string& account_id) const;

        //订阅行情
        //@param source_id   柜台ID
        //@param instruments 合约列表
        //@param exchange_id 交易所ID
        //@param is_level2   是否订阅Level2数据
        void subscribe(const std::string& source_id, const std::vector<std::string>& instruments, const std::string& exchange_id = "", bool is_level2 = false);

        //限价单报单
        //@param instrument_id 合约ID
        //@param exchange_id   交易所ID
        //@param account_id    账户ID
        //@param limit_price   价格
        //@param volume        数量
        //@param side          买卖方向
        //@param offset        开平方向
        //@return              订单ID
        uint64_t insert_limit_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                    double limit_price, int64_t volume, Side side, Offset offset);
        //fok单报单
        //@param instrument_id 合约ID
        //@param exchange_id   交易所ID
        //@param account_id    账户ID
        //@param limit_price   价格
        //@param volume        数量
        //@param side          买卖方向
        //@param offset        开平方向
        //@return              订单ID
        uint64_t insert_fok_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                  double limit_price, int64_t volume, Side side, Offset offset);
        //fak单报单
        //@param instrument_id 合约ID
        //@param exchange_id   交易所ID
        //@param account_id    账户ID
        //@param limit_price   价格
        //@param volume        数量
        //@param side          买卖方向
        //@param offset        开平方向
        //@return              订单ID
        uint64_t insert_fak_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                  double limit_price, int64_t volume, Side side, Offset offset);
        //市价单报单
        //@param instrument_id 合约ID
        //@param exchange_id   交易所ID
        //@param account_id    账户ID
        //@param volume        数量
        //@param side          买卖方向
        //@param offset        开平方向
        //@return              订单ID
        uint64_t insert_market_order(const std::string& instrument_id, const std::string& exchange_id, const std::string& account_id,
                                     int64_t volume, Side side, Offset offset);

        //撤单
        //@param order_id      订单ID
        //@return              撤单操作ID
        uint64_t cancel_order(uint64_t order_id);

        //注册时间回调函数
        //@param nano          纳秒时间
        //@param callback      回调函数
        void register_nanotime_callback(int64_t nano, std::function<void (int64_t)> callback);

        //算法订单报单
        //@param algo_type     算法订单类型
        //@param input         算法订单输入信息
        //@return              订单ID
        uint64_t insert_algo_order(const std::string& algo_type, const std::string& input);

        //修改算法订单
        //@param order_id      订单ID
        //@param action        算法订单修改信息
        //@return              订单操作ID
        uint64_t modify_algo_order(uint64_t order_id, const std::string& action);

        //尝试冻结请求,如果返回 0，表示尝试失败，否则返回操作ID
        //@param account_id    账户ID
        //@param frozen        冻结请求内容
        //@return              操作ID
        uint64_t try_frozen(const std::string& account_id, const flying::AssetsFrozen& frozen);

        //尝试冻结请求，如果返回 true,表示尝试成功，否则表示失败
        //@param op_id         操作ID
        //@param account_id    账户ID
        //@param frozen        冻结请求内容
        //@return              成功或者失败
        bool try_frozen(uint64_t op_id, const std::string& account_id, const flying::AssetsFrozen& frozen);

        //取消冻结请求
        //@param op_id         操作ID
        //@param account_id    账号ID
        void cancel_frozen(uint64_t op_id, const std::string& account_id);

        //提交冻结
        //@param op_id         操作ID
        //@param account_id    账号ID
        bool commit_frozen(uint64_t op_id, const std::string& account_id);

        //尝试修改策略在具体某个账户下的持仓，返回0，表示尝试失败，否则返回操作ID
        //@param account_id    账户ID
        //@param pos_modify    持仓修改
        uint64_t try_modify_position(const std::string& account_id, const flying::PositionModify& pos_modify);

        //取消修改持仓
        //@param op_id         操作ID
        //@param account_id    账号ID
        void cancel_modify_position(uint64_t op_id, const std::string& account_id);

        //提交修改持仓请求，如果返回 true,表示尝试成功，否则表示失败
        //@param op_id         操作ID
        //@param pos_modify    持仓修改
        //@return              成功或者失败
        bool commit_modify_position(uint64_t op_id, const std::string& account_id);

        //初始化接
        virtual void init() {};

        //运行前
        virtual void pre_run() {};

        //退出前
        virtual void pre_quit() {};

        //交易日切换回调
        //@param next_trading_day  下一个交易日
        virtual void on_switch_day(const std::string& next_trading_day) {};

        //行情数据更新回调
        //@param quote             行情数据
        virtual void on_quote(const journal::Quote& quote) {};

        //逐笔委托更新回调
        //@param entrust           逐笔委托数据
        virtual void on_entrust(const journal::Entrust& entrust) {};

        //逐笔成交更新回调
        //@param transaction       逐笔成交数据
        virtual void on_transaction(const journal::Transaction& transaction) {};

        //订单信息更新回调
        //@param order             订单信息数据
        virtual void on_order(const journal::Order& order) {};

        //订单成交回报回调
        //@param trade             订单成交数据
        virtual void on_trade(const journal::Trade& trade) {};

        //算法订单状态更新回调
        //@param order_id          订单ID
        //@param algo_type         订单类型
        //@param event_msg         订单更新事件
        virtual void on_algo_order_status(uint64_t order_id, const std::string& algo_type, const std::string& event_msg) {};

    private:
        class impl;
        std::unique_ptr<impl> impl_;
        std::string name_;
    };
}
#endif //WC_2_STRATEGY_H
