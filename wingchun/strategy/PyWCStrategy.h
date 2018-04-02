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
 * PyWCStrategy: python binding version of wingchun strategy.
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   October, 2017
 */

#ifndef WINGCHUN_PYWCSTRATEGY_H
#define WINGCHUN_PYWCSTRATEGY_H

#include "IWCStrategy.h"

WC_NAMESPACE_START

class PyWCStrategy: public IWCStrategy
{
private:
    boost::python::object py_init;
    boost::python::object py_on_switch_day;
    boost::python::object py_on_pos;
    boost::python::object py_on_error;
    map<short, boost::python::object> py_on_data;

public:
    virtual void start();
    virtual void init();
    virtual void on_market_bar(const BarMdMap& data, int min_interval, short source, long rcv_time);
    virtual void on_market_data(const LFMarketDataField* data, short source, long rcv_time);
    virtual void on_rtn_order(const LFRtnOrderField* data, int request_id, short source, long rcv_time);
    virtual void on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time);
    virtual void on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr);
    virtual void on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time);
    virtual void on_switch_day(long rcv_time);
    //py log
    virtual void log_debug(string msg) { KF_LOG_DEBUG(logger, msg); };
    virtual void log_info(string msg) { KF_LOG_INFO(logger, msg); };
    virtual void log_error(string msg) { KF_LOG_ERROR(logger, msg); };
    virtual void log_fatal(string msg) { KF_LOG_FATAL(logger, msg); };
    //set py function
    void set_init(boost::python::object func);
    void set_on_data(short msg_type, boost::python::object func);
    void set_on_pos(boost::python::object func);
    void set_on_error(boost::python::object func);
    void set_on_switch_day(boost::python::object func);

    WCStrategyUtilPtr get_strategy_util() const { return util; }
    WCDataWrapperPtr  get_data_wrapper()  const { return data; }
    /** get effective orders */
    boost::python::list get_effective_orders() const;

public:
    // python binding, no char allowed, use string and transfer manually.
    inline int insert_market_order_py(short source, string instrument_id, string exchange_id, int volume, string direction, string offset)
    {
        return insert_market_order(source, instrument_id, exchange_id, volume, direction[0], offset[0]);
    }
    inline int insert_limit_order_py(short source, string instrument_id, string exchange_id, double price, int volume, string direction, string offset)
    {
        return insert_limit_order(source, instrument_id, exchange_id, price, volume, direction[0], offset[0]);
    }
    inline int insert_fok_order_py(short source, string instrument_id, string exchange_id, double price, int volume, string direction, string offset)
    {
        return insert_fok_order(source, instrument_id, exchange_id, price, volume, direction[0], offset[0]);
    }
    inline int insert_fak_order_py(short source, string instrument_id, string exchange_id, double price, int volume, string direction, string offset)
    {
        return insert_fak_order(source, instrument_id, exchange_id, price, volume, direction[0], offset[0]);
    }

public:
    PyWCStrategy(const string& name): IWCStrategy(name) {}
    PyWCStrategy(): IWCStrategy("Default") {}
};

DECLARE_PTR(PyWCStrategy);

WC_NAMESPACE_END

#endif //WINGCHUN_PYWCSTRATEGY_H
