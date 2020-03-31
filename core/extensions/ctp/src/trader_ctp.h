//
// Created by qlu on 2019/1/14.
//

#ifndef KUNGFU_CTP_EXT_TRADER_H
#define KUNGFU_CTP_EXT_TRADER_H

#include <kungfu/longfist/longfist.h>
#include <kungfu/wingchun/broker/trader.h>
#include <kungfu/yijinjing/common.h>

#include "common.h"

#include "ThostFtdcTraderApi.h"

namespace kungfu::wingchun::ctp {
typedef std::unordered_map<std::string, longfist::types::Position> PositionMap;
typedef std::unordered_map<std::string, longfist::types::Instrument> InstrumentMap;

class TraderCTP : public CThostFtdcTraderSpi, public broker::Trader {
public:
  TraderCTP(bool low_latency, yijinjing::data::locator_ptr locator, const std::string &account_id,
            const std::string &json_config);

  ~TraderCTP() override = default;

  longfist::enums::AccountType get_account_type() const override { return longfist::enums::AccountType::Future; }

  bool insert_order(const event_ptr &event) override;

  bool cancel_order(const event_ptr &event) override;

  bool req_position() override;

  bool req_account() override;

  virtual bool req_position_detail();

  virtual void OnFrontConnected();

  virtual void OnFrontDisconnected(int nReason);

  virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField,
                                 CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

  virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo,
                              int nRequestID, bool bIsLast);

  virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID,
                               bool bIsLast);

  virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo,
                                int nRequestID, bool bIsLast);

  virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo,
                                int nRequestID, bool bIsLast);

  virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

  virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

  virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition,
                                        CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

  virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
                                              CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

  virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo,
                                      int nRequestID, bool bIsLast);

  virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo,
                                  int nRequestID, bool bIsLast);

  virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
                                          CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

protected:
  void on_start() override;

private:
  TDConfiguration config_;

  int front_id_;
  int session_id_;

  int order_ref_;
  int request_id_;

  CThostFtdcTraderApi *api_;

  std::unordered_map<std::string, uint64_t> inbound_order_refs_;
  std::unordered_map<std::string, uint64_t> inbound_order_sysids_;
  std::unordered_map<std::string, uint64_t> inbound_trade_ids_;
  std::unordered_map<uint64_t, uint64_t> inbound_actions_;
  std::unordered_map<uint64_t, uint64_t> outbound_orders_;

  PositionMap long_position_map_;
  PositionMap short_position_map_;

  InstrumentMap instrument_map_;

  std::unordered_map<int, longfist::types::OrderAction> action_event_map_;

  std::string trading_day_;

  bool login();

  bool req_settlement_confirm();

  bool req_auth();

  bool req_qry_instrument();
};
} // namespace kungfu::wingchun::ctp
#endif // KUNGFU_CTP_EXT_TRADER_H
