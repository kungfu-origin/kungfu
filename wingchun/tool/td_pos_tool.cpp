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
 * Tool for td position show and set.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   Oct, 2017
 */

#include "TDUserStruct.h"
#include "Timer.h"
#include "PosHandler.hpp"
#include "longfist/LFConstants.h"
#include "longfist/LFUtils.h"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

using namespace boost::program_options;
using kungfu::yijinjing::PosHandlerPtr;

WC_NAMESPACE_START

#define TIME_FORMAT "%Y%m%d-%H:%M:%S"

        vector<string> split_text(string line)
{
string buff {""};
vector<string> v;
for (auto n: line)
{
if (n != ',' && n != ' ')
{
buff += n;
}
else if ((n == ',' || n == ' ') && buff != "")
{
v.push_back(buff);
buff = "";
}
}
if (buff.length() > 0)
v.push_back(buff);
return v;
}

void fill_data(PosHandlerPtr pos, vector<string>& data, int indice[])
{
    if (indice[0] >= 0 && indice[1] >= 0)
    {
        pos->set_pos(data[0], LF_CHAR_Net, std::stoi(data[indice[0]]), std::stoi(data[indice[1]]),
                     (indice[6] >= 0) ? std::stod(data[indice[6]]) : 0.0,
                     (indice[7] >= 0) ? std::stod(data[indice[7]]) : 0.0);
    }
    if (indice[2] >= 0 && indice[3] >= 0)
    {
        pos->set_pos(data[0], LF_CHAR_Long, std::stoi(data[indice[2]]), std::stoi(data[indice[3]]),
                     (indice[8] >= 0) ? std::stod(data[indice[8]]) : 0.0,
                     (indice[9] >= 0) ? std::stod(data[indice[9]]) : 0.0);
    }
    if (indice[4] >= 0 && indice[5] >= 0)
    {
        pos->set_pos(data[0], LF_CHAR_Short, std::stoi(data[indice[4]]), std::stoi(data[indice[5]]),
                     (indice[10] >= 0) ? std::stod(data[indice[10]]) : 0.0,
                     (indice[11] >= 0) ? std::stod(data[indice[11]]) : 0.0);
    }
}

bool set_pos_tool(const string& user_name, short source, const string& file_name, bool is_csv)
{
    std::ifstream my_file(file_name);
    if (!my_file.is_open())
    {
        std::cout << "[ERROR] cannot open file: " << file_name << std::endl;
        return false;
    }
    TDUserInfoHelperPtr helper = TDUserInfoHelperPtr(new TDUserInfoHelper(source));
    helper->load(user_name, true);
    json js;

    if (is_csv)
    {
        PosHandlerPtr pos = kungfu::yijinjing::PosHandler::create(source);
        // start to parse csv file
        string line;
        vector<string> data;
        bool started = false;
        int number_idx[] = { -1/*net_tot*/, -1/*net_yd*/,
                             -1/*long_tot*/, -1/*long_yd*/,
                             -1/*short_tot*/, -1/*short_yd*/,
                             -1/*net_amt*/, -1/*net_fee*/,
                             -1/*long_amt*/, -1/*long_fee*/,
                             -1/*short_amt*/, -1/*short_fee*/};
        while (std::getline(my_file, line))
        {
            data = split_text(line);
            if (!started)
            {
                char c = data[1][0];
                if (c <= '9' && c >= '0')
                {
                    // no header line
                    if (data.size() == 7)
                    {
                        for (int i = 0; i < 6; i++)
                        {
                            number_idx[i] = i+1;
                        }
                        fill_data(pos, data, number_idx);
                    }
                    else
                    {
                        std::cout << "column types unrecognized!" << std::endl;
                    }
                }
                else
                {
                    for (size_t i = 1; i < data.size(); i++)
                    {
                        string column = data[i];
                        if (column.compare("net_tot") == 0)
                            number_idx[0] = i;
                        else if (column.compare("net_yd") == 0)
                            number_idx[1] = i;
                        else if (column.compare("long_tot") == 0)
                            number_idx[2] = i;
                        else if (column.compare("long_yd") == 0)
                            number_idx[3] = i;
                        else if (column.compare("short_tot") == 0)
                            number_idx[4] = i;
                        else if (column.compare("short_yd") == 0)
                            number_idx[5] = i;
                        else if (column.compare("net_balance") == 0)
                            number_idx[6] = i;
                        else if (column.compare("net_fee") == 0)
                            number_idx[7] = i;
                        else if (column.compare("long_balance") == 0)
                            number_idx[8] = i;
                        else if (column.compare("long_fee") == 0)
                            number_idx[9] = i;
                        else if (column.compare("short_balance") == 0)
                            number_idx[10] = i;
                        else if (column.compare("short_fee") == 0)
                            number_idx[11] = i;
                    }
                }
                started = true;
            }
            else
            {
                fill_data(pos, data, number_idx);
            }
        }
        js = pos->to_json();
        std::cout << "set pos from csv file: " << file_name << std::endl;
    }
    else
    {
        std::string contents((std::istreambuf_iterator<char>(my_file)),
                             std::istreambuf_iterator<char>());
        js = json::parse(contents);
        std::cout << "set pos from json file: " << file_name << std::endl;
    }
    js["is_manual"] = true;
    helper->set_pos(user_name, js);
    return true;
}

bool get_pos_tool(const string& user_name, short source, bool print_raw, bool need_detail, bool print_file, bool is_csv, const string& output_file)
{
    std::cout << "=========== (name)" << user_name << " (source)" << source << " =========" << std::endl;
    TDUserInfoHelperPtr helper = TDUserInfoHelperPtr(new TDUserInfoHelper(source, user_name));
    if (need_detail)
    {
        // print detail if need
        auto info = helper->get_const_user_info(user_name);
        if (info != nullptr)
        {
            std::cout << "status:\t[" << info->status << "]" << std::endl;
            std::cout << "start_time:\t" << info->start_time
                      << " (" << yijinjing::parseNano(info->start_time, TIME_FORMAT) << ")"
                      << std::endl;
            std::cout << "end_time:\t" << info->end_time
                      << " (" << yijinjing::parseNano(info->end_time, TIME_FORMAT) << ")"
                      << std::endl;

            std::cout << "last_order_index:\t" << info->last_order_index << std::endl;
            std::cout << "name:\t" << info->name << std::endl;
            std::cout << "========" << std::endl;
            std::cout << "orders:" << std::endl;
            for (int i = 0; i <= info->last_order_index; i++)
            {
                const TDOrderInfo& order = info->orders[i];
                if (order.status != ORDER_INFO_RAW)
                {
                    std::cout << "\t(order_id)" << order.order_id
                              << " (local_id)" << order.local_id
                              << " (ticker)" << order.ticker
                              << " (status)" << order.status << std::endl;
                }
            }
            std::cout << "========" << std::endl;
        }
    }
    json js = helper->get_pos(user_name);

    if (print_raw || js.find("Source") == js.end() || js.find("Pos") == js.end())
    {
        std::cout << js.dump() << std::endl;
        std::cout << "-------" << std::endl;
    }
    PosHandlerPtr pos = kungfu::yijinjing::PosHandler::create(source, js.dump());
    auto tickers = pos->get_tickers();
    if (js.find("nano") != js.end())
    {
        long nano = js["nano"].get<long>();
        std::cout << "nano:\t" << nano << " (" << yijinjing::parseNano(nano, TIME_FORMAT) << ")" << std::endl;
    }
    else
    {
        std::cout << "nano:\tNone" << std::endl;
    }
    if (js.find("ok") != js.end())
    {
        if (js["ok"].get<bool>())
            std::cout << "ok:\tYes" << std::endl;
        else
            std::cout << "ok:\tNo" << std::endl;
    }
    else
    {
        std::cout << "ok:\tNone" << std::endl;
    }
    std::cout << "-------" << std::endl;
    for (auto ticker: tickers)
    {
        std::cout << ticker;
        std::cout << "\t(net)" << pos->get_net_total(ticker) << "," << pos->get_net_yestd(ticker)
                  << "\t(long)" << pos->get_long_total(ticker) << "," << pos->get_long_yestd(ticker)
                  << "\t(short)" << pos->get_short_total(ticker) << "," << pos->get_short_yestd(ticker)
                  << "\t(net_c)" << pos->get_net_balance(ticker) << "," << pos->get_net_fee(ticker)
                  << "\t(long_c)" << pos->get_long_balance(ticker) << "," << pos->get_long_fee(ticker)
                  << "\t(short_c)" << pos->get_short_balance(ticker) << "," << pos->get_short_fee(ticker)
                  << std::endl;
    }
    std::cout << "-------" << std::endl;

    if (print_file)
    {
        std::ofstream of;
        of.open(output_file, std::ios::out | std::ios::trunc | std::ios::binary);
        if (of.is_open())
        {
            if (is_csv)
            {
                of << "ticker, net_tot, net_yd, long_tot, long_yd, short_tot, short_yd, net_amt, net_fee, long_amt, long_fee, short_amt, short_fee\n";
                for (auto ticker: tickers)
                {
                    of << ticker << ", " << pos->get_net_total(ticker) << ", " << pos->get_net_yestd(ticker)
                       << ", " << pos->get_long_total(ticker) << ", " << pos->get_long_yestd(ticker)
                       << ", " << pos->get_short_total(ticker) << ", " << pos->get_short_yestd(ticker)
                       << ", " << pos->get_net_balance(ticker) << ", " << pos->get_net_fee(ticker)
                       << ", " << pos->get_long_balance(ticker) << ", " << pos->get_long_fee(ticker)
                       << ", " << pos->get_short_balance(ticker) << ", " << pos->get_short_fee(ticker) << std::endl;
                }
                std::cout << "printed to csv file: " << output_file << std::endl;
            }
            else
            {
                of << js.dump() << std::endl;
                std::cout << "printed to json file: " << output_file << std::endl;
            }
            of.close();
            std::cout << "-------" << std::endl;
        }
        else
        {
            std::cout << "[ERROR] cannot open file: " << output_file << std::endl;
            return false;
        }
    }
    return true;
}

WC_NAMESPACE_END

void list_user_folder()
{
    if (!boost::filesystem::exists(TD_USER_INFO_FOLDER))
    {
        std::cout << "base folder does not exists! " << TD_USER_INFO_FOLDER << std::endl;
    }
    else
    {
        boost::filesystem::directory_iterator end_itr; // default construction yields past-the-end
        vector<string> unrelated_files;
        std::cout << "+-------------------------------+" << std::endl;
        std::cout << "|          TD Pos Files         |" << std::endl;
        std::cout << "+----------------+--------------+" << std::endl;
        std::cout << "| strategy_name  | source       |" << std::endl;
        std::cout << "+----------------+--------------+" << std::endl;
        for ( boost::filesystem::directory_iterator itr( TD_USER_INFO_FOLDER );
              itr != end_itr;
              ++itr )
        {
            if (boost::filesystem::is_regular_file(itr->status()))
            {
                string filename = itr->path().filename().string();
                int idx = filename.find('.');
                if (idx > 0)
                {
                    try
                    {
                        short source;
                        string td_name;
                        source = (short)std::stoi(filename.substr(idx + 1));
                        JournalPair jp = getTdJournalPair(source);
                        if (jp.second.length() > 0)
                        {
                            printf("|%16s|%11s %2d|\n", filename.substr(0, idx).c_str(), jp.second.c_str(), source);
                            std::cout << "+----------------+--------------+" << std::endl;
                        }
                        else
                        {
                            unrelated_files.push_back(filename);
                        }
                    }
                    catch(...)
                    {
                        unrelated_files.push_back(filename);
                    }

                }
            }
        }
        if (unrelated_files.size() > 0)
        {
            std::cout << "Unexpected files in folder " << TD_USER_INFO_FOLDER << ":\n";
            for (auto fn: unrelated_files)
                std::cout << "\t" << fn << std::endl;
        }
    }
}

USING_WC_NAMESPACE
/**
 * main entrance
 */
int main(int argc, const char* argv[])
{
    options_description desc{"Options"};
    desc.add_options()
            ("help,h", "Help screen")
            ("type,t", value<string>(), "type (\"set\"/\"get\")")
            ("source,s", value<int>(), "source index (eg CTP=1)")
            ("name,n", value<string>(), "strategy name")
            ("output,o", "output to file")
            ("raw,r", "print raw data")
            ("detail,d", "print user info details")
            ("list,l", "list all files")
            ("json,j", value<string>(), "json_file")
            ("csv,c", value<string>(), "csv_file format:\nticker, net_tot, net_yd, long_tot, long_yd, short_tot, short_yd");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("list"))
    {
        list_user_folder();
        return 0;
    }
    bool need_help = vm.count("help");

    if (need_help)
    {
        std::cout << desc << std::endl;
        return 0;
    }

    short source = -1;
    string type = "";
    string name = "";
    string csv_file = "";
    string json_file = "";

    if (vm.count("source"))
    {
        source = vm["source"].as<int>();
    }
    if (vm.count("type"))
    {
        type = vm["type"].as<string>();
    }
    if (vm.count("name"))
    {
        name = vm["name"].as<string>();
    }
    if (vm.count("csv"))
    {
        csv_file = vm["csv"].as<string>();
    }
    if (vm.count("json"))
    {
        json_file = vm["json"].as<string>();
    }

    // filter out
    if (name.length() == 0)
    {
        std::cout << "no available name" << std::endl;
        need_help = true;
    }
    if (source < 0)
    {
        std::cout << "no available source" << std::endl;
        need_help = true;
    }

    if (type.compare("set") == 0)
    {
        if (csv_file.length() == 0 && json_file.length() == 0)
        {
            std::cout << "no available file" << std::endl;
            need_help = true;
        }
    }
    else if (type.compare("get") == 0)
    {
        if (vm.count("output") > 0 && csv_file.length() == 0 && json_file.length() == 0)
        {
            std::cout << "no available file" << std::endl;
            need_help = true;
        }
    }
    else
    {
        std::cout << "no allowed type (set/get) is determined. (" << type << ")" << std::endl;
        need_help = true;
    }

    // do something
    if (need_help)
    {
        std::cout << desc << std::endl;
    }
    else
    {
        if (type.compare("set") == 0)
        {
            if (vm.count("csv") > 0)
            {
                set_pos_tool(name, source, csv_file, true);
            }
            if (vm.count("json") > 0)
            {
                set_pos_tool(name, source, json_file, false);
            }
        }
        else
        {
            if (vm.count("output") > 0)
            {
                if (vm.count("csv") > 0)
                {
                    get_pos_tool(name, source, vm.count("raw") > 0, vm.count("detail") > 0, true, true, csv_file);
                }
                if (vm.count("json") > 0)
                {
                    get_pos_tool(name, source, vm.count("raw") > 0, vm.count("detail") > 0, true, false, json_file);
                }

            }
            // get mode
            get_pos_tool(name, source, vm.count("raw") > 0, vm.count("detail") > 0, false, false, "");
        }
    }
}