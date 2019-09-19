//
// Created by Keren Dong on 2019-07-11.
//

#ifndef EXT_PASSIVE_TRADER_H
#define EXT_PASSIVE_TRADER_H

#include <kungfu/wingchun/broker/trader.h>
#include <kungfu/wingchun/common.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace sim
        {
            enum class MatchMode
            {
                Reject,
                Pend,
                Cancel,
                PartialFillAndCancel,
                PartialFill,
                Fill,
                Custom
            };

            inline MatchMode get_mode(const std::string& m)
            {
                auto m_lower = to_lower_copy(m);
                if (m_lower == "reject")
                { return MatchMode::Reject;}
                else if (m_lower == "pend")
                { return MatchMode::Pend; }
                else if (m_lower == "cancel")
                { return MatchMode::Cancel;}
                else if (m_lower == "partialfillandcancel")
                { return MatchMode::PartialFillAndCancel;}
                else if (m_lower == "partialfill")
                { return MatchMode::PartialFill; }
                else if (m_lower == "fill")
                { return MatchMode::Fill;}
                else
                { return MatchMode::Custom;}
            }

            class TraderSim : public broker::Trader
            {
            public:
                TraderSim(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &account_id, const std::string &json_config);

                const AccountType get_account_type() const override { return AccountType::Stock; };

                bool insert_order(const yijinjing::event_ptr &event) override;

                bool cancel_order(const yijinjing::event_ptr &event) override;

                bool req_position() override { return false;};

                bool req_account() override { return false;};

                void on_start() override
                {
                    broker::Trader::on_start();
                    publish_state(msg::data::BrokerState::Ready);
                }

            private:
                MatchMode mode_;
                std::unordered_map<uint64_t, uint32_t> oid2source_;
                std::unordered_map<uint64_t, msg::data::Order> pending_orders_;
            };
        }
    }
}

#endif //EXT_PASSIVE_TRADER_H
