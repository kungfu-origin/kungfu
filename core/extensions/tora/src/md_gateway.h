//
// Created by PolarAir on 2019-06-18.
//

#ifndef KUNGFU_MD_GATEWAY_H
#define KUNGFU_MD_GATEWAY_H

#include <kungfu/wingchun/gateway/gateway_impl.h>
#include "TORATstpMdApi.h"

namespace kungfu
{
    namespace tora
    {
        class MdGateWay : public CTORATstpMdSpi, public kungfu::MdGatewayImpl
        {
        public:
            MdGateWay(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double);
            ~MdGateway();

            void init() override ;

            bool subscribe(const std::vector<Instrument>& instruments, bool is_level2 = false) override;
            bool unsubscribe(const std::vector<Instrument>& instruments) override;

            ///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
            void OnFrontConnected() override;

            ///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
            ///        -3 连接已断开
            ///        -4 网络读失败
            ///        -5 网络写失败
            ///        -6 订阅流错误
            ///        -7 流序号错误
            ///        -8 错误的心跳报文
            ///        -9 错误的报文
            void OnFrontDisconnected(int nReason) override;

            ///错误应答
            void OnRspError(CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

            ///登录请求响应
            void OnRspUserLogin(CTORATstpRspUserLoginField *pRspUserLogin, CTORATstpRspInfoField *pRspInfo, int nRequestID, bool bIsLast) override;

            ///深度行情通知
            void OnRtnDepthMarketData(CTORATstpMarketDataField *pDepthMarketData) override;

        private:
            void login();
            int get_req_id() const;

        private:
            std::string             front_address_;
            std::string             user_id_;
            std::string             password_;

            CTORATstpMdApi*         api_;
        };
    }
}

#endif //KUNGFU_MD_GATEWAY_H
