#ifndef BAR
#define BAR

#include <string>
#include <vector>
#include <map>

#include <kungfu/wingchun/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/practice/apprentice.h>

namespace kungfu
{
    namespace wingchun
    {
        class BarHandle: public practice::apprentice
        {
        public:
            BarHandle(yijinjing::data::location_ptr home, const std::string &source, int frequency, bool low_latency = false);
            void register_bar(int source, int frequency, int64_t start_time);
            void snapshot_updata(const kungfu::wingchun::msg::data::Quote &quote, uint32_t source);
            void send_bar(const msg::data::Bar &bar);
        
        private:
            void inline new_bar(msg::data::Bar &bar, const kungfu::wingchun::msg::data::Quote &quote);
            void react() override;
            void on_start() override;
            void test();

        private:    
            std::map<std::string, msg::data::Bar> bar_map;
            std::string _source;
            int _frequency;
        };
    }
}

#endif //BAR