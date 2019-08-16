//
// Created by Keren Dong on 2019-07-11.
//

#ifndef EXT_PASSIVE_MARKETDATA_H
#define EXT_PASSIVE_MARKETDATA_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/marketdata.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace sim
        {

            enum DataType
            {
                Static,
                StandardSine,
                StandardLine,
                StandardRandom
            };

            struct MdParameter
            {
                char instrument_id[INSTRUMENT_ID_LEN];
                DataType type;
                double cycle;
                double limit;
                double pre_price;
                int interval = 1;
            };

            class MarketDataSim : public broker::MarketData
            {
            public:
                MarketDataSim(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &json_config);

                bool subscribe(const std::vector<msg::data::Instrument> &instruments) override;

                bool unsubscribe(const std::vector<msg::data::Instrument> &instruments) override;

            private:
                void on_start() override;

                bool init_md();

                void creat_md();

                void static_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp);

                void sin_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp);

                void line_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp);

                void random_quota(msg::data::Quote &quote, const int &time, const MdParameter &mdp);

                int get_inter_val(char *instrument_id);

            private:
                std::vector<MdParameter> md_parameters_;
                std::map<std::string, msg::data::Quote> mds_;
            };
        }
    }
}

#endif //EXT_PASSIVE_MARKETDATA_H
