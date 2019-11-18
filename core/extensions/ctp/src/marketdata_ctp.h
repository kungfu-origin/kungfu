//
// Created by qlu on 2019/1/10.
//

#ifndef KUNGFU_CTP_EXT_MARKETDATA_H
#define KUNGFU_CTP_EXT_MARKETDATA_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/marketdata.h>

#include "common.h"

#include "ThostFtdcMdApi.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace ctp
        {
            class MarketDataCTP : public CThostFtdcMdSpi, public broker::MarketData
            {
            public:
                MarketDataCTP(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &json_config);

                virtual ~MarketDataCTP()
                {};

                bool subscribe(const std::vector<wingchun::msg::data::Instrument> &instruments) override;

                bool unsubscribe(const std::vector<wingchun::msg::data::Instrument> &instruments) override;

                bool subscribe_all() override;

                bool subscribe(const std::vector<std::string> &instrument_ids);

                bool unsubscribe(const std::vector<std::string> &instrument_ids);

                bool login();

                ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
                virtual void OnFrontConnected();

                ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
                ///@param nReason 错误原因
                ///        0x1001 网络读失败
                ///        0x1002 网络写失败
                ///        0x2001 接收心跳超时
                ///        0x2002 发送心跳失败
                ///        0x2003 收到错误报文
                virtual void OnFrontDisconnected(int nReason);

                ///登录请求响应
                virtual void
                OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                ///登出请求响应
                virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                ///订阅行情应答
                virtual void
                OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                   bool bIsLast);

                ///取消订阅行情应答
                virtual void
                OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                     bool bIsLast);

                ///深度行情通知
                virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

            protected:

                void on_start() override;

            private:
                Configuration config_;
                int request_id_;
                CThostFtdcMdApi *api_;
            };
        }
    }
}
#endif //KUNGFU_CTP_EXT_MARKETDATA_H
