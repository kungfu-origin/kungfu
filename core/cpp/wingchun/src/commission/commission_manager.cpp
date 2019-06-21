//
// Created by PolarAir on 2019-04-13.
//

#include <kungfu/wingchun/config.h>
#include <kungfu/wingchun/storage/storage.h>
#include <kungfu/wingchun/util/business_helper.h>
#include <kungfu/wingchun/commission/commission_manager.h>

using namespace kungfu::wingchun::msg::data;
using namespace kungfu::wingchun::msg::nanomsg;

namespace kungfu
{
    namespace wingchun
    {

        CommissionManager::CommissionManager(const std::string &account_id, const std::string& db_file) :
                simple_default_{}, stock_rate_{}, storage_(account_id, db_file)
        {
            storage_.load(stock_rate_, future_rates_);
            storage_.register_callback(std::bind(&CommissionManager::on_commission, this, std::placeholders::_1));
        }

        const InstrumentCommissionRate &CommissionManager::get_commission_rate(const std::string &instrument_id,
                                                                               const std::string &exchange_id) const
        {
            auto instrument_type = get_instrument_type(instrument_id, exchange_id);
            if (instrument_type == InstrumentTypeStock || instrument_type == InstrumentTypeBond)
            {
                return stock_rate_;
            } else if (instrument_type == InstrumentTypeFuture)
            {
                if (future_rates_.find(instrument_id) != future_rates_.end())
                {
                    return future_rates_.at(instrument_id);
                } else
                {
                    auto product = get_instrument_product(instrument_id);
                    if (future_rates_.find(product) != future_rates_.end())
                    {
                        return future_rates_.at(product);
                    } else if (future_rates_.find("") != future_rates_.end())
                    {
                        return future_rates_.at("");
                    } else
                    {
                        return simple_default_;
                    }
                }
            } else
            {
                return simple_default_;
            }
        }


        double CommissionManager::calc_commission(const OrderInput *input) const
        {
            auto rate = get_commission_rate(input->instrument_id, input->exchange_id);
            double base = rate.mode == CommissionRateModeByAmount ? input->frozen_price * input->volume : input->volume;
            if (input->instrument_type == InstrumentTypeStock || input->instrument_type == InstrumentTypeBond)
            {
                return std::max(base * (input->side == SideBuy ? rate.open_ratio : rate.close_ratio), rate.min_commission);
            } else if (input->instrument_type == InstrumentTypeFuture)
            {
                double ratio = rate.open_ratio;
                if (input->offset == OffsetClose || input->offset == OffsetCloseYesterday)
                {
                    ratio = rate.close_ratio;
                } else if (input->offset == OffsetCloseToday)
                {
                    ratio = rate.close_today_ratio;
                }
                return std::max(base * ratio, rate.min_commission);
            } else
            {
                return 0.0;
            }
        }

        void CommissionManager::update_commission(const InstrumentCommissionRate &commission)
        {
            storage_.save(commission);
        }

        void CommissionManager::update_commissions(const std::vector<InstrumentCommissionRate> &commissions)
        {
            storage_.save(commissions);
        }

        void CommissionManager::on_commission(const InstrumentCommissionRate &commission)
        {
            if (commission.instrument_type == InstrumentTypeStock || commission.instrument_type == InstrumentTypeFuture)
            {
                memcpy(&stock_rate_, &commission, sizeof(InstrumentCommissionRate));
            } else if (commission.instrument_type == InstrumentTypeFuture)
            {
                memcpy(&future_rates_[commission.instrument_id], &commission, sizeof(InstrumentCommissionRate));
            }
        }

    }
}