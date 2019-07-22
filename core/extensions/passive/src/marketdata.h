//
// Created by Keren Dong on 2019-07-11.
//

#ifndef EXT_PASSIVE_MARKETDATA_H
#define EXT_PASSIVE_MARKETDATA_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/marketdata.h>

namespace kungfu
{
    namespace wingchun
    {
        namespace passive
        {
            
            enum DataType
            {
                StandardSine,
                StandardLine,
                StandardRandom,
                GusssRandom
            };
            struct MdParameter
            {
                char InstrumentId[INSTRUMENT_ID_LEN];
                double Cycle;
                double Limit;
                double PrePrice;
                int Interval = 1;
            };

            class PassiveMarketData : public gateway::MarketData
            {
            public:
                PassiveMarketData(bool low_latency, yijinjing::data::locator_ptr locator, std::map<std::string, std::string> &config_str,
                                  std::map<std::string, int> &config_int,
                                  std::map<std::string, double> &config_double);

                bool subscribe(const std::vector<msg::data::Instrument> &instruments) override ;
                bool unsubscribe(const std::vector<msg::data::Instrument> &instruments) override ;

            private:
                void on_start() override;
                bool init_md();

                void create_md();
                void sin_quota(msg::data::Quote &quote, int time, MdParameter parameter);
            
            private:
                std::map<DataType, std::vector<MdParameter>> md_parameters;
                std::map<std::string, msg::data::Quote> mds;
            };
        }
    }
}

#endif //EXT_PASSIVE_MARKETDATA_H
