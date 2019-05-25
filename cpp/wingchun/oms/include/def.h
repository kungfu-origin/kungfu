//
// Created by qlu on 2019/3/1.
//

#ifndef KUNGFU_DEF_H
#define KUNGFU_DEF_H

#include <nlohmann/json.hpp>
#include <cstring>
#include <unordered_map>
#include <string>

#include <kungfu/wingchun/oms_struct.h>
#include <kungfu/wingchun/md_struct.h>

namespace kungfu
{
    namespace oms
    {
        class OmsOrder
        {
        public:
            virtual uint64_t get_order_id() const = 0;
            virtual void execute() = 0;
            virtual void cancel() = 0;
            virtual bool is_algo() const = 0;
            virtual bool is_active() const = 0;
        };
        typedef std::shared_ptr<kungfu::oms::OmsOrder> OrderPtr;

        class SimpleOrderUtil;

        class SimpleOrder: public OmsOrder
        {
        public:
            virtual uint64_t get_order_id() const = 0;
            virtual std::string get_account_id() const = 0;
            virtual std::string get_client_id() const = 0;

            virtual void execute() = 0;
            virtual void cancel() = 0;

            virtual bool is_algo() const = 0;
            virtual bool is_active() const = 0;

            virtual OrderStatus get_status() const = 0;
            virtual bool is_children_order() const = 0;
            virtual uint64_t get_parent_id() const = 0;

            virtual void set_util(SimpleOrderUtil* util) = 0;
            virtual SimpleOrderUtil* get_util() const = 0;

            virtual void on_order(const journal::Order *order) = 0;
        };

        typedef std::shared_ptr<kungfu::oms::SimpleOrder> SimpleOrderPtr;
        SimpleOrderPtr make_simple_order(const journal::OrderInput& input);
        SimpleOrderPtr make_simple_order(const journal::Order& order);

        class SimpleOrderUtil
        {
        public:
            virtual uint64_t submit(journal::OrderInput& input) = 0;
            virtual uint64_t cancel(uint64_t order_id) = 0;
        };

        // 算法订单工具类
        class AlgoOrderUtil
        {
        public:
            typedef std::function<void (long)> TSCallback;
            //推送算法订单状态
            //@param order_id （必须输入）算法订单id
            //@param algo_type (必须输入) 算法订单类型
            //@param event (必须输入) 算法订单事件类型
            //@param event_msg (必须输入) 算法订单事件消息
            virtual void send_order_status(uint64_t order_id, const std::string& algo_type, const std::string& event) = 0;

            //发送子订单
            //@param order_id (必须输入) 算法订单id
            //@param child_order_input （必须输入） 子订单信息
            //@return 子订单id，如果为0，表示发送失败
            virtual uint64_t submit_child_order(uint64_t order_id, journal::OrderInput& child_order_input) = 0;

            //保存算法订单信息
            //@param order_id（必须输入）算法订单id
            //@param algo_type（必须输入）算法订单类型
            //@param order_info (必须输入) 算法订单信息
            virtual void save(uint64_t order_id, const std::string& algo_type, const std::string& order_info) = 0;

            //尝试冻结请求接口，如果返回 true,表示尝试成功，否则表示失败
            virtual bool try_frozen(uint64_t order_id, const std::string& account_id, const flying::AssetsFrozen& frozen) { return false;};

            //取消冻结请求接口
            virtual void cancel_frozen(uint64_t order_id, const std::string& account_id) {};

            //提交冻结接口
            virtual bool commit_frozen(uint64_t order_id, const std::string& account_id) { return false; };

            //获取当前纳秒时间
            virtual long get_nano() const = 0;

            //注册时间回调函数
            virtual void register_nanotime_callback(long nano, TSCallback callback) = 0;

            //获取时间区间内的 trading session
            //@param exchange_id 交易所ID
            //@param start_nano 开始纳秒时间戳
            //@param end_nano 结束纳秒时间戳
            //@return 交易所交易时段
            virtual std::vector<kungfu::flying::TradingSession> get_trading_sessions(long start_nano, long end_nano, const std::string& exchange_id) = 0;

        };

        //算法订单基类
        class AlgoOrder: public OmsOrder
        {
        public:
            AlgoOrder();
            virtual ~AlgoOrder();

            //获取算法订单工具对象指针
            //@return 算法订单工具对象指针
            AlgoOrderUtil* get_util() const;

            //设置算法订单工具
            //@param util （必须输入）算法订单工具类的实例
            void set_util(AlgoOrderUtil* util);

            //设置订单id
            //@param order_id 算法订单id
            //@remark order_id 必须为系统内unique id
            void set_order_id(uint64_t order_id);

            //获取订单id
            //@return 算法订单id
            uint64_t get_order_id() const;

            //是否是算法订单
            bool is_algo() const { return true; };

            //获取算法类型
            //@return 算法订单类型
            virtual std::string get_algo_type() const = 0;

            //订单是否活跃
            virtual bool is_active() const = 0;

            //执行算法订单方法
            virtual void execute() = 0;

            //撤销算法订单方法
            virtual void cancel() = 0;

            //算法订单修改请求
            //@param cmd 算法订单修改请求命令
            virtual void modify(const std::string& cmd) = 0;

            //子订单状态更新通知
            //@param order 子订单通知具体信息
            //@remark 子订单状态更新，函数会被调用
            virtual void on_child_order(const journal::Order& order) = 0;

            //子订单成交通知
            //@param trade 子订单成交通知具体信息
            //@remark 子订单有成交，函数会被调用
            virtual void on_child_trade(const journal::Trade& trade) = 0;

            //行情更新通知
            virtual void on_quote(const journal::Quote& quote) = 0;

            //时间更新通知
            virtual void on_time(long nano) = 0;

            //算法订单初始化方法
            virtual void init(const std::string& order_input) = 0;

            //算法订单序列化方法
            virtual std::string dumps() const = 0;

            //算法订单反序列化方法
            virtual void loads(const std::string& order_info) = 0;

        private:
            class impl;
            std::unique_ptr<impl> impl_;
        };

        typedef std::shared_ptr<kungfu::oms::AlgoOrder> AlgoOrderPtr;
        typedef AlgoOrderPtr (*CreateAlgoOrderFn)(void);

        //算法订单工厂类
        class AlgoOrderFactory
        {
        public:
            ~AlgoOrderFactory()
            {
                create_funcs_.clear();
            }

            static AlgoOrderFactory *get()
            {
                static AlgoOrderFactory instance;
                return &instance;

            }

            //注册算法订单类型
            template <typename AlgoOrderDerived>
            void register_type(std::string& algo_type)
            {
                static_assert(std::is_base_of<AlgoOrder, AlgoOrderDerived>::value, "AlgoOrderFactory::register_type doesn't accept this type because doesn't derive from AlgoOrder");
                create_funcs_[algo_type] = &createFunc<AlgoOrderDerived>;
            }

            //创建算法订单
            AlgoOrderPtr create_order(const std::string& algo_type)
            {
                typename std::unordered_map<std::string,PCreateFunc>::const_iterator it = create_funcs_.find(algo_type);
                if (it != create_funcs_.end())
                {
                    return it->second();
                }
                return nullptr;
            }

        private:
            template <typename AlgoOrderDerived>
            static std::shared_ptr<AlgoOrder> createFunc()
            {
                return std::shared_ptr<AlgoOrder>(new AlgoOrderDerived());
            }

            typedef AlgoOrderPtr (*PCreateFunc)();
            std::unordered_map<std::string,PCreateFunc> create_funcs_;

            AlgoOrderFactory() {};
            AlgoOrderFactory(const AlgoOrderFactory &) {}
            AlgoOrderFactory &operator=(const AlgoOrderFactory &) { return *this; }
        };

        class OrderManager
        {
        public:
            virtual void on_order(const kungfu::journal::Order* order) = 0;
            virtual void on_algo_order_status(uint64_t order_id, const std::string& algo_type, const std::string& order_status) = 0;
            virtual OrderPtr get_order(uint64_t order_id) const = 0;
            virtual std::vector<OrderPtr> get_pending_orders() const = 0;
        };

        typedef std::shared_ptr<OrderManager> OrderManagerPtr;
        OrderManagerPtr create_order_manager();

    }
}
#endif //KUNGFU_DEF_H
