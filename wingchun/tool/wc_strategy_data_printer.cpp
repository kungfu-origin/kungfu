/*****************************************************************************
 * Copyright [2017] [taurus.ai]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

/**
 * Tool for print out strategy trading data.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   Nov, 2017
 */

#include "StrategyDataConsumer.h"
#include "Timer.h"
#include <boost/program_options.hpp>

using namespace boost::program_options;
USING_WC_NAMESPACE

void list_strategy_sessions(const string& name, long start_nano, long end_nano, int index)
{
    StrategyBasicDataConsumer consumer(name, index);
    consumer.init(start_nano, end_nano);
    consumer.run();
}

void show_order_trades(const string& name, long start_nano, long end_nano, int index, bool show_order, bool show_trade, bool show_state, bool show_detail)
{
    StrategyDataConsumer consumer(name, index);
    consumer.init(start_nano, end_nano);
    consumer.run();
    if (show_order)
        consumer.print_order();
    if (show_trade)
        consumer.print_trade();
    if (show_state)
        consumer.print_stat();
    if (show_detail)
        consumer.print_detail();
}

/**
 * main entrance
 */
int main(int argc, const char* argv[])
{
    options_description desc{"Options"};
    string format("%Y%m%d-%H:%M:%S");
    desc.add_options()
            ("help,h", "Help screen")
            ("name,n", value<string>(), "strategy name")
            ("list,l", "list all sessions with index")
            ("order,o", "show orders")
            ("trade,t", "show trades")
            ("aggregate,a", "show aggregated latency stat")
            ("detail,d", "list order detail nanos")
            ("index,i", value<int>()->default_value(-1), "session index")
            ("start_time,s", value<string>()->default_value("20000101-13:30:00"), "start time (%Y%m%d-%H:%M:%S)")
            ("end_time,e", value<string>()->default_value("20200101-00:00:00"), "end time (%Y%m%d-%H:%M:%S)");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help"))
    {
        std::cout << desc << std::endl;
        return 0;
    }

    long sn = kungfu::yijinjing::parseTime(vm["start_time"].as<string>().c_str(), format.c_str());
    long en = kungfu::yijinjing::parseTime(vm["end_time"].as<string>().c_str(), format.c_str());
    if (!vm.count("name"))
    {
        std::cout << " please specify strategy name!! " << std::endl;
        return -1;
    }
    string name = vm["name"].as<string>();
    int idx = vm["index"].as<int>();
    bool show_order = vm.count("order");
    bool show_trade = vm.count("trade");
    bool show_stat = vm.count("aggregate");
    bool show_detail = vm.count("detail");

    if (vm.count("list") || ((!show_order) && (!show_trade) && (!show_stat) && (!show_detail)))
    {
        list_strategy_sessions(name, sn, en, idx);
        return 0;
    }
    show_order_trades(name, sn, en, idx, show_order, show_trade, show_stat, show_detail);
}