//
// Created by qlu on 2019/1/16.
//

#ifndef WC_2_EVENT_LOOP_H
#define WC_2_EVENT_LOOP_H

#include <memory>
#include <csignal>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/io.h>

#include <kungfu/wingchun/md_struct.h>
#include <kungfu/wingchun/oms_struct.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/util/task_scheduler.h>

namespace kfj = kungfu::journal;

namespace kungfu
{
    typedef std::function<void (const kfj::Quote& quote)> QuoteCallback;
    typedef std::function<void (const kfj::Entrust& entrust)> EntrustCallback;
    typedef std::function<void (const kfj::Transaction& transaction)> TransactionCallback;
    typedef std::function<void (const std::string& recipient, std::vector<kfj::Instrument>&, bool is_level2)> SubscribeCallback;
    typedef std::function<void (const std::string& recipient, const std::string& client_id)> ReqLoginCallback;
    typedef std::function<void (const kfj::OrderInput& input)> OrderInputCallback;
    typedef std::function<void (const kfj::OrderAction& action)> OrderActionCallback;
    typedef std::function<void (const kfj::Order& order)> OrderCallback;
    typedef std::function<void (const kfj::Trade& trade)> TradeCallback;
    typedef std::function<void (uint64_t order_id, const std::string& client_id, const std::string& algo_type, const std::string& algo_order_input)> AlgoOrderInputCallback;
    typedef std::function<void (uint64_t order_id, const std::string& algo_type, const std::string& status)> AlgoOrderStatusCallback;
    typedef std::function<void (uint64_t order_id, uint64_t order_action_id, const std::string& cmd)> AlgoOrderActionCallback;
    typedef std::function<void (kfj::OrderInput& input)> ManualOrderInputCallback;
    typedef std::function<void (const std::string& account_id, const std::string& client_id, const std::vector<uint64_t>& order_ids)> ManualOrderActionCallback;
    typedef std::function<void ()> ReloadInstrumentsCallback;
    typedef std::function<void (const std::string&)> SwitchDayCallback;

    class EventLoop: public kungfu::yijinjing::event_handler
    {
    public:
        EventLoop(): scheduler_(new TaskScheduler()) {};

        void register_nanotime_callback(int64_t nano, TSCallback callback); // if nano == 0, trigger at next update
        void register_nanotime_callback_at_next(const char* time_str, TSCallback callback);

        void register_quote_callback(QuoteCallback callback);
        void register_entrust_callback(EntrustCallback callback);
        void register_transaction_callback(TransactionCallback callback);

        void register_order_input_callback(OrderInputCallback callback);
        void register_order_action_callback(OrderActionCallback callback);

        void register_order_callback(OrderCallback callback);
        void register_trade_callback(TradeCallback callback);

        void register_subscribe_callback(SubscribeCallback callback);
        void register_req_login_callback(ReqLoginCallback callback);

        void register_algo_order_input_callback(AlgoOrderInputCallback callback);
        void register_algo_order_status_callback(AlgoOrderStatusCallback callback);
        void register_algo_order_action_callback(AlgoOrderActionCallback callback);

        void register_manual_order_input_callback(ManualOrderInputCallback callback);
        void register_manual_order_action_callback(ManualOrderActionCallback callback);

        void register_reload_instruments_callback(ReloadInstrumentsCallback callback);

        void register_switch_day_callback(SwitchDayCallback cb);

        int64_t get_nano() const;

        void handle(const yijinjing::event *e) override ;

    private:

        std::unique_ptr<TaskScheduler> scheduler_;

        QuoteCallback quote_callback_;
        EntrustCallback entrust_callback_;
        TransactionCallback transaction_callback_;

        ReqLoginCallback login_callback_;
        SubscribeCallback sub_callback_;
        OrderInputCallback order_input_callback_;
        OrderActionCallback order_action_callback_;
        OrderCallback order_callback_;
        TradeCallback trade_callback_;

        AlgoOrderInputCallback algo_order_input_callback_;
        AlgoOrderStatusCallback algo_order_status_callback_;
        AlgoOrderActionCallback algo_order_action_callback_;

        ManualOrderInputCallback manual_order_input_callback_;
        ManualOrderActionCallback manual_order_action_callback_;

        ReloadInstrumentsCallback reload_instruments_callback_;
        SwitchDayCallback switch_day_callback_;
    };
    DECLARE_PTR(EventLoop)
}
#endif //WC_2_EVENT_LOOP_H
