//
// Created by PolarAir on 2019-04-13.
//

#include <kungfu/wingchun/commission/commission_manager.h>
#include <kungfu/wingchun/commission/commission_storage.h>
#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/util/business_helper.h>

using namespace kungfu::journal;
using namespace kungfu::flying;

namespace kungfu
{
    class CommissionManager::impl
    {
    public:
        explicit impl(const char* account_id) : simple_default_{}, stock_rate_{}, storage_(new CommissionStorage(account_id))
        {
            storage_->load(stock_rate_, future_rates_);
            storage_->register_callback(std::bind(&CommissionManager::impl::on_commission, this, std::placeholders::_1));
        }

        ~impl()
        {
            if (nullptr != storage_)
            {
                delete storage_;
                storage_ = nullptr;
            }
        }

        const InstrumentCommissionRate* get_commission_rate(const char *instrument_id,
                const char *exchange_id) const
        {
            auto instrument_type = get_instrument_type(instrument_id, exchange_id);
            if (instrument_type == InstrumentTypeStock || instrument_type == InstrumentTypeBond)
            {
                return &stock_rate_;
            }
            else if (instrument_type == InstrumentTypeFuture)
            {
                if (future_rates_.find(instrument_id) != future_rates_.end())
                {
                    return &future_rates_.at(instrument_id);
                }
                else
                {
                    auto product = get_instrument_product(instrument_id);
                    if (future_rates_.find(product) != future_rates_.end())
                    {
                        return &future_rates_.at(product);
                    }
                    else if (future_rates_.find("") != future_rates_.end())
                    {
                        return &future_rates_.at("");
                    }
                    else
                    {
                        return &simple_default_;
                    }
                }
            }
            else
            {
                return &simple_default_;
            }
        }

        void update_commission(const InstrumentCommissionRate &commission)
        {
            storage_->save(commission);
        }

        void udpate_commissions(const vector<InstrumentCommissionRate> &commissions)
        {
            storage_->save(commissions);
        }

        void on_commission(const InstrumentCommissionRate &commission)
        {
            if (commission.instrument_type == InstrumentTypeStock || commission.instrument_type == InstrumentTypeFuture)
            {
                memcpy(&stock_rate_, &commission, sizeof(InstrumentCommissionRate));
            }
            else if (commission.instrument_type == InstrumentTypeFuture)
            {
                memcpy(&future_rates_[commission.instrument_id], &commission, sizeof(InstrumentCommissionRate));
            }
        }

    private:
        InstrumentCommissionRate simple_default_;
        InstrumentCommissionRate stock_rate_;
        std::map<std::string, InstrumentCommissionRate> future_rates_;
        CommissionStorage* storage_;
    };

    CommissionManager::CommissionManager(const char *account_id) : impl_(new impl(account_id))
    {

    }

    CommissionManager::~CommissionManager()
    {
        if (nullptr != impl_)
        {
            delete impl_;
            impl_ = nullptr;
        }
    }

    const InstrumentCommissionRate* CommissionManager::get_commission_rate(const char *instrument_id,
            const char *exchange_id) const
    {
        return impl_->get_commission_rate(instrument_id, exchange_id);
    }

    double CommissionManager::calc_commission(const OrderInput *input) const
    {
        auto rate = get_commission_rate(input->instrument_id, input->exchange_id);
        double base = rate->mode == CommissionRateModeByAmount ? input->frozen_price * input->volume : input->volume;
        if (input->instrument_type == InstrumentTypeStock || input->instrument_type == InstrumentTypeBond)
        {
            return std::max(base * (input->side == SideBuy ? rate->open_ratio : rate->close_ratio), rate->min_commission);
        }
        else if (input->instrument_type == InstrumentTypeFuture)
        {
            double ratio = rate->open_ratio;
            if (input->offset == OffsetClose || input->offset == OffsetCloseYesterday)
            {
                ratio = rate->close_ratio;
            }
            else if (input->offset == OffsetCloseToday)
            {
                ratio = rate->close_today_ratio;
            }
            return std::max(base * ratio, rate->min_commission);
        }
        else
        {
            return 0.0;
        }
    }

    void CommissionManager::update_commission(const InstrumentCommissionRate &commission)
    {
        impl_->update_commission(commission);
    }

    void CommissionManager::update_commissions(const std::vector<InstrumentCommissionRate> &commissions)
    {
        impl_->udpate_commissions(commissions);
    }
}