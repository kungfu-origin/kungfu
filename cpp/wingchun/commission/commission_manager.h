//
// Created by PolarAir on 2019-04-13.
//

#ifndef KUNGFU_COMMISSION_MANAGER_H
#define KUNGFU_COMMISSION_MANAGER_H

#include "oms_struct.h"
#include <memory>

namespace kungfu
{
    class CommissionManager
    {
    public:
        explicit CommissionManager(const char* account_id);
        ~CommissionManager();

        const InstrumentCommissionRate* get_commission_rate(const char* instrument_id, const char* exchange_id) const;

        double calc_commission(const OrderInput* input) const;

        void update_commission(const InstrumentCommissionRate& commission);
        void update_commissions(const std::vector<InstrumentCommissionRate>& commissions);

    private:
        class impl;
        impl* impl_;
    };
    typedef std::shared_ptr<CommissionManager> CommissionManagerPtr;
}

#endif //KUNGFU_COMMISSION_MANAGER_H
