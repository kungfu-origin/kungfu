//
// Created by qlu on 2019/1/14.
//

#ifndef KUNGFU_CTP_EXT_TRADER_H
#define KUNGFU_CTP_EXT_TRADER_H

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/broker/trader.h>

#include "common.h"
#include "order_mapper.h"
#include "trade_mapper.h"

#include "ThostFtdcTraderApi.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace ctp
        {
            typedef std::shared_ptr<OrderMapper> OrderMapperPtr;
            typedef std::shared_ptr<TradeMapper> TradeMapperPtr;
            typedef std::unordered_map<std::string, msg::data::Position> PositionMap;
            typedef std::unordered_map<std::string, msg::data::Instrument> InstrumentMap;

            class TraderCTP : public CThostFtdcTraderSpi, public broker::Trader
            {
            public:
                TraderCTP(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &account_id, const std::string &json_config);

                ~TraderCTP() {};

                const AccountType get_account_type() const override
                { return AccountType::Future; }

                bool insert_order(const yijinjing::event_ptr& event) override;

                bool cancel_order(const yijinjing::event_ptr& event) override;

                bool req_position() override;

                bool req_account() override;

                virtual bool req_position_detail() ;

                virtual void OnFrontConnected();

                virtual void OnFrontDisconnected(int nReason);

                virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                virtual void
                OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                virtual void
                OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

                virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

                virtual void
                OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                         bool bIsLast);

                virtual void
                OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo,
                                               int nRequestID, bool bIsLast);

                virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                                                    bool bIsLast);

                virtual void
                OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

                virtual void
                OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo,
                                           int nRequestID, bool bIsLast);
            protected:

                void on_start() override;

            private:
                Configuration config_;

                int front_id_;
                int session_id_;

                int order_ref_;
                int request_id_;

                CThostFtdcTraderApi *api_;

                OrderMapperPtr order_mapper_;
                TradeMapperPtr trade_mapper_;

                PositionMap long_position_map_;
                PositionMap short_position_map_;

                InstrumentMap instrument_map_;

                std::unordered_map<int, msg::data::OrderAction> action_event_map_;

                std::string trading_day_;

                bool login();

                bool req_settlement_confirm();

                bool req_auth();

                bool req_qry_instrument();

            };
        }
    }
}
#endif //KUNGFU_CTP_EXT_TRADER_H
