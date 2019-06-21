//
// Created by PolarAir on 2019-04-13.
//

#ifndef KUNGFU_COMMISSION_MANAGER_H
#define KUNGFU_COMMISSION_MANAGER_H

#include <kungfu/wingchun/msg.h>

namespace kungfu
{
    namespace wingchun
    {
        class CommissionManager
        {
        public:
            explicit CommissionManager(const std::string& account_id, const std::string& db_file);

            ~CommissionManager();

            const msg::data::InstrumentCommissionRate& get_commission_rate(const std::string& instrument_id, const std::string& exchange_id) const;

            double calc_commission(const msg::data::OrderInput *input) const;

            void update_commission(const msg::data::InstrumentCommissionRate &commission);

            void update_commissions(const std::vector<msg::data::InstrumentCommissionRate> &commissions);

            void on_commission(const msg::data::InstrumentCommissionRate &commission);

        private:

        private:
            msg::data::InstrumentCommissionRate simple_default_;
            msg::data::InstrumentCommissionRate stock_rate_;
            std::map<std::string, msg::data::InstrumentCommissionRate> future_rates_;
            storage::CommissionStorage storage_;
        };
    }
}

#endif //KUNGFU_COMMISSION_MANAGER_H
