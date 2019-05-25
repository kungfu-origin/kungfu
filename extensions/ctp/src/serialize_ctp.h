//
// Created by qlu on 2019/3/21.
//

#ifndef KUNGFU_SERIALIZE_H
#define KUNGFU_SERIALIZE_H

#include <string>
#include <nlohmann/json.hpp>
#include <kungfu/wingchun/util/code_convert.h>
#include "ThostFtdcUserApiStruct.h"

namespace kungfu
{
    namespace ctp
    {
        inline void to_json(nlohmann::json& j, const CThostFtdcInputOrderField& ori)
        {
            j["BrokerID"] = std::string(ori.BrokerID);
            j["InvestorID"] = std::string(ori.InvestorID);
            j["InstrumentID"] = std::string(ori.InstrumentID);
            j["OrderRef"] = std::string(ori.OrderRef);
            j["UserID"] = std::string(ori.UserID);
            j["OrderPriceType"] = std::string(1, ori.OrderPriceType);
            j["Direction"] = std::string(1, ori.Direction);
            j["CombOffsetFlag"] = std::string(ori.CombOffsetFlag);
            j["CombHedgeFlag"] = std::string(ori.CombHedgeFlag);
            j["LimitPrice"] = ori.LimitPrice;
            j["VolumeTotalOriginal"] = ori.VolumeTotalOriginal;
            j["TimeCondition"] = std::string(1, ori.TimeCondition);
            j["GTDDate"] = std::string(ori.GTDDate);
            j["VolumeCondition"] = std::string(1, ori.VolumeCondition);
            j["MinVolume"] = ori.MinVolume;
            j["ContingentCondition"] = std::string(1, ori.ContingentCondition);
            j["StopPrice"] = ori.StopPrice;
            j["ForceCloseReason"] = std::string(1, ori.ForceCloseReason);
            j["IsAutoSuspend"] = ori.IsAutoSuspend;
            j["BusinessUnit"] = ori.BusinessUnit;
            j["RequestID"] = ori.RequestID;
            j["UserForceClose"] = ori.UserForceClose;
            j["IsSwapOrder"] = ori.IsSwapOrder;
#ifdef __linux__
            j["ExchangeID"] = std::string(ori.ExchangeID);
            j["InvestUnitID"] = std::string(ori.InvestUnitID);
            j["AccountID"] = std::string(ori.AccountID);
            j["CurrencyID"] = std::string(ori.CurrencyID);
            j["ClientID"] = std::string(ori.ClientID);
            j["IPAddress"] = std::string(ori.IPAddress);
            j["MacAddress"] = std::string(ori.MacAddress);
#endif
        }

         inline void to_json(nlohmann::json& j, const CThostFtdcOrderField& ori)
        {
            j["BrokerID"] = std::string(ori.BrokerID);
            j["InvestorID"] = std::string(ori.InvestorID);
            j["InstrumentID"] = std::string(ori.InstrumentID);
            j["OrderRef"] = std::string(ori.OrderRef);
            j["UserID"] = std::string(ori.UserID);
            j["OrderPriceType"] = std::string(1, ori.OrderPriceType);
            j["Direction"] = std::string(1, ori.Direction);
            j["CombOffsetFlag"] = std::string(ori.CombOffsetFlag);
            j["CombHedgeFlag"] = std::string(ori.CombHedgeFlag);
            j["LimitPrice"] = ori.LimitPrice;
            j["VolumeTotalOriginal"] = ori.VolumeTotalOriginal;
            j["TimeCondition"] = std::string(1, ori.TimeCondition);
            j["GTDDate"] = std::string(ori.GTDDate);
            j["VolumeCondition"] = std::string(1,ori.VolumeCondition);
            j["MinVolume"] = ori.MinVolume;
            j["ContingentCondition"] =  std::string(1,ori.ContingentCondition);
            j["StopPrice"] = ori.StopPrice;
            j["ForceCloseReason"] = std::string(1, ori.ForceCloseReason);
            j["IsAutoSuspend"] = ori.IsAutoSuspend;
            j["BusinessUnit"] = std::string(ori.BusinessUnit);
            j["RequestID"] = ori.RequestID;
            j["OrderLocalID"] = std::string(ori.OrderLocalID);
            j["ExchangeID"] = std::string(ori.ExchangeID);
            j["ParticipantID"] = std::string(ori.ParticipantID);
            j["ClientID"] = std::string(ori.ClientID);
            j["TThostFtdcExchangeInstIDType"] = std::string(ori.ExchangeInstID);
            j["TraderID"] = std::string(ori.TraderID);
            j["InstallID"] = ori.InstallID;
            j["OrderSubmitStatus"] = std::string(1, ori.OrderSubmitStatus);
            j["NotifySequence"] = ori.NotifySequence;
            j["TradingDay"] = std::string(ori.TradingDay);
            j["SettlementID"] = ori.SettlementID;
            j["OrderSysID"] = std::string(ori.OrderSysID);
            j["OrderSource"] = std::string(1, ori.OrderSource);
            j["OrderStatus"] = std::string(1, ori.OrderStatus);
            j["OrderType"] = std::string(1, ori.OrderType);
            j["VolumeTraded"] = ori.VolumeTraded;
            j["VolumeTotal"] = ori.VolumeTotal;
            j["InsertDate"] = std::string(ori.InsertDate);
            j["InsertTime"] = std::string(ori.InsertTime);
            j["ActiveTime"] = std::string(ori.ActiveTime);
            j["SuspendTime"] = std::string(ori.SuspendTime);
            j["UpdateTime"] = std::string(ori.UpdateTime);
            j["CancelTime"] = std::string(ori.CancelTime);
            j["ActiveTraderID"] = std::string(ori.ActiveTraderID);
            j["ClearingPartID"] = std::string(ori.ClearingPartID);
            j["SequenceNo"] = ori.SequenceNo;
            j["FrontID"] = ori.FrontID;
            j["SessionID"] = ori.SessionID;
            j["UserProductInfo"] = std::string(ori.UserProductInfo);
            j["StatusMsg"] = gbk2utf8(ori.StatusMsg);
            j["UserForceClose"] = ori.UserForceClose;
            j["ActiveUserID"] = std::string(ori.ActiveUserID);
            j["BrokerOrderSeq"] = ori.BrokerOrderSeq;
            j["RelativeOrderSysID"] = std::string(ori.RelativeOrderSysID);
            j["ZCETotalTradedVolume"] = ori.ZCETotalTradedVolume;
            j["IsSwapOrder"] = ori.IsSwapOrder;
#ifdef __linux__
            j["BranchID"] = std::string(ori.BranchID);
            j["InvestUnitID"] = std::string(ori.InvestUnitID);
            j["AccountID"] = std::string(ori.AccountID);
            j["CurrencyID"] = std::string(ori.CurrencyID);
            j["IPAddress"] = std::string(ori.IPAddress);
            j["MacAddress"] = std::string(ori.MacAddress);
#endif
        }

        inline void to_json(nlohmann::json& j, const CThostFtdcTradingAccountField& ori)
        {
            j["BrokerID"] = std::string(ori.BrokerID);
            j["AccountID"] = std::string(ori.AccountID);
            j["PreMortgage"] = ori.PreMortgage;
            j["PreCredit"] = ori.PreCredit;
            j["PreDeposit"] = ori.PreDeposit;
            j["PreBalance"] = ori.PreBalance;
            j["PreMargin"] = ori.PreMargin;
            j["InterestBase"] = ori.InterestBase;
            j["Interest"] = ori.Interest;
            j["Deposit"] = ori.Deposit;
            j["Withdraw"] = ori.Withdraw;
            j["FrozenMargin"] = ori.FrozenMargin;
            j["FrozenCash"] = ori.FrozenCash;
            j["FrozenCommission"] = ori.FrozenCommission;
            j["CurrMargin"] = ori.CurrMargin;
            j["CashIn"] = ori.CashIn;
            j["Commission"] = ori.Commission;
            j["CloseProfit"] = ori.CloseProfit;
            j["PositionProfit"] = ori.PositionProfit;
            j["Balance"] = ori.Balance;
            j["Available"] = ori.Available;
            j["WithdrawQuota"] = ori.WithdrawQuota;
            j["Reserve"] = ori.Reserve;
            j["TradingDay"] = std::string(ori.TradingDay);
            j["SettlementID"] = ori.SettlementID;
            j["Credit"] = ori.Credit;
            j["Mortgage"] = ori.Mortgage;
            j["ExchangeMargin"] = ori.ExchangeMargin;
            j["DeliveryMargin"] = ori.DeliveryMargin;
            j["ExchangeDeliveryMargin"] = ori.ExchangeDeliveryMargin;
            j["ReserveBalance"] = ori.ReserveBalance;
            j["CurrencyID"] = std::string(ori.CurrencyID);
            j["PreFundMortgageIn"] = ori.PreFundMortgageIn;
            j["PreFundMortgageOut"] = ori.PreFundMortgageOut;
            j["FundMortgageIn"] = ori.FundMortgageIn;
            j["FundMortgageOut"] = ori.FundMortgageOut;
            j["FundMortgageAvailable"] = ori.FundMortgageAvailable;
            j["MortgageableFund"] = ori.MortgageableFund;
            j["SpecProductMargin"] = ori.SpecProductMargin;
            j["SpecProductFrozenMargin"] = ori.SpecProductFrozenMargin;
            j["SpecProductCommission"] = ori.SpecProductCommission;
            j["SpecProductFrozenCommission"] = ori.SpecProductFrozenCommission;
            j["SpecProductPositionProfit"] = ori.SpecProductPositionProfit;
            j["SpecProductCloseProfit"] = ori.SpecProductCloseProfit;
            j["SpecProductPositionProfitByAlg"] = ori.SpecProductPositionProfitByAlg;
            j["SpecProductExchangeMargin"] = ori.SpecProductExchangeMargin;
        }

        inline void to_json(nlohmann::json& j, const CThostFtdcInvestorPositionField& ori)
        {
            j["InstrumentID"] = std::string(ori.InstrumentID);
            j["BrokerID"] = std::string(ori.BrokerID);
            j["InvestorID"] = std::string(ori.InvestorID);
            j["PosiDirection"] = std::string(1, ori.PosiDirection);
            j["HedgeFlag"] = std::string(1, ori.HedgeFlag);
            j["PositionDate"] = std::string(1, ori.PositionDate);
            j["YdPosition"] = ori.YdPosition;
            j["Position"] = ori.Position;
            j["LongFrozen"] = ori.LongFrozen;
            j["ShortFrozen"] = ori.ShortFrozen;
            j["LongFrozenAmount"] = ori.LongFrozenAmount;
            j["ShortFrozenAmount"] = ori.ShortFrozenAmount;
            j["OpenVolume"] = ori.OpenVolume;
            j["CloseVolume"] = ori.CloseVolume;
            j["OpenAmount"] = ori.OpenAmount;
            j["CloseAmount"] = ori.CloseAmount;
            j["PositionCost"] = ori.PositionCost;
            j["PreMargin"] = ori.PreMargin;
            j["UseMargin"] = ori.UseMargin;
            j["FrozenMargin"] = ori.FrozenMargin;
            j["FrozenCash"] = ori.FrozenCash;
            j["FrozenCommission"] = ori.FrozenCommission;
            j["CashIn"] = ori.CashIn;
            j["Commission"] = ori.Commission;
            j["CloseProfit"] = ori.CloseProfit;
            j["PositionProfit"] = ori.PositionProfit;
            j["PreSettlementPrice"] = ori.PreSettlementPrice;
            j["SettlementPrice"] = ori.SettlementPrice;
            j["TradingDay"] = ori.TradingDay;
            j["SettlementID"] = ori.SettlementID;
            j["OpenCost"] = ori.OpenCost;
            j["ExchangeMargin"] = ori.ExchangeMargin;
            j["CombPosition"] = ori.CombPosition;
            j["CombLongFrozen"] = ori.CombLongFrozen;
            j["CombShortFrozen"] = ori.CombShortFrozen;
            j["CloseProfitByDate"] = ori.CloseProfitByDate;
            j["CloseProfitByTrade"] = ori.CloseProfitByTrade;
            j["TodayPosition"] = ori.TodayPosition;
            j["MarginRateByMoney"] = ori.MarginRateByMoney;
            j["MarginRateByVolume"] = ori.MarginRateByVolume;
            j["StrikeFrozen"] = ori.StrikeFrozen;
            j["StrikeFrozenAmount"] = ori.StrikeFrozenAmount;
            j["AbandonFrozen"] = ori.AbandonFrozen;
        }

        inline void to_json(nlohmann::json& j, const CThostFtdcInstrumentField& ori)
        {
            j["InstrumentID"] = std::string(ori.InstrumentID);
            j["ExchangeID"] = std::string(ori.ExchangeID);
            j["InstrumentName"] = gbk2utf8(ori.InstrumentName);
            j["ExchangeInstID"] = std::string(ori.ExchangeInstID);
            j["ProductID"] = gbk2utf8(ori.ProductID);
            j["ProductClass"] = std::string(1, ori.ProductClass);
            j["DeliveryYear"] = ori.DeliveryYear;
            j["DeliveryMonth"] = ori.DeliveryMonth;
            j["MaxMarketOrderVolume"] = ori.MaxMarketOrderVolume;
            j["MinMarketOrderVolume"] = ori.MinMarketOrderVolume;
            j["MaxLimitOrderVolume"] = ori.MaxLimitOrderVolume;
            j["MinLimitOrderVolume"] = ori.MinLimitOrderVolume;
            j["VolumeMultiple"] = ori.VolumeMultiple;
            j["PriceTick"] = ori.PriceTick;
            j["CreateDate"] = std::string(ori.CreateDate);
            j["OpenDate"] = std::string(ori.OpenDate);
            j["ExpireDate"] = std::string(ori.ExpireDate);
            j["StartDelivDate"] = std::string(ori.StartDelivDate);
            j["EndDelivDate"] = std::string(ori.EndDelivDate);
            j["InstLifePhase"] = std::string(1, ori.InstLifePhase);
            j["IsTrading"] = ori.IsTrading;
            j["PositionType"] = std::string(1, ori.PositionType);
            j["PositionDateType"] = std::string(1, ori.PositionDateType);
            j["LongMarginRatio"] = ori.LongMarginRatio;
            j["ShortMarginRatio"] = ori.ShortMarginRatio;
            j["MaxMarginSideAlgorithm"] = ori.MaxMarginSideAlgorithm;
            j["UnderlyingInstrID"] = ori.UnderlyingInstrID;
            j["StrikePrice"] = ori.StrikePrice;
            j["OptionsType"] = std::string(1, ori.OptionsType);
            j["UnderlyingMultiple"] = ori.UnderlyingMultiple;
            j["CombinationType"] = std::string(1, ori.CombinationType);
        }

        inline void to_json(nlohmann::json& j, const CThostFtdcInvestorPositionDetailField& ori)
        {
            j["InstrumentID"] = std::string(ori.InstrumentID ? ori.InstrumentID :"");
            j["BrokerID"] = ori.BrokerID == nullptr ? "": std::string(ori.BrokerID);
            j["InvestorID"] = ori.InvestorID == nullptr ? "" : std::string(ori.InvestorID);
            j["HedgeFlag"] = std::string(1, ori.HedgeFlag);
            j["Direction"] = std::string(1, ori.Direction);
            j["OpenDate"] = ori.OpenDate == nullptr ? "" : std::string(ori.OpenDate);
            j["TradeID"] = ori.TradeID == nullptr ? "" : std::string(ori.TradeID);
            j["Volume"] = ori.Volume;
            j["OpenPrice"] = ori.OpenPrice;
            j["TradingDay"] = ori.TradingDay == nullptr ? "" : std::string(ori.TradingDay);
            j["SettlementID"] = ori.SettlementID;
            j["TradeType"] = std::string(1, ori.TradeType);
            j["CombInstrumentID"] = ori.CombInstrumentID == nullptr ? "" : std::string(ori.CombInstrumentID);
            j["ExchangeID"] = ori.ExchangeID == nullptr ? "" : std::string(ori.ExchangeID);
            j["CloseProfitByDate"] = ori.CloseProfitByDate;
            j["CloseProfitByTrade"] = ori.CloseProfitByTrade;
            j["PositionProfitByDate"] = ori.PositionProfitByDate;
            j["PositionProfitByTrade"] = ori.PositionProfitByTrade;
            j["Margin"] = ori.Margin;
            j["ExchMargin"] = ori.ExchMargin;
            j["MarginRateByMoney"] = ori.MarginRateByMoney;
            j["MarginRateByVolume"] = ori.MarginRateByVolume;
            j["LastSettlementPrice"] = ori.LastSettlementPrice;
            j["SettlementPrice"] = ori.SettlementPrice;
            j["CloseVolume"] = ori.CloseVolume;
            j["CloseAmount"] = ori.CloseAmount;
        }

        inline void to_json(nlohmann::json& j, const CThostFtdcDepthMarketDataField& ori)
        {
            j["TradingDay"] = std::string(ori.TradingDay);
            j["InstrumentID"] = std::string(ori.InstrumentID);
            j["ExchangeID"] = std::string(ori.ExchangeID);
            j["ExchangeInstID"] = std::string(ori.ExchangeInstID);
            j["LastPrice"] = ori.LastPrice;
            j["PreSettlementPrice"] = ori.PreSettlementPrice;
            j["PreClosePrice"] = ori.PreClosePrice;
            j["PreOpenInterest"] = ori.PreOpenInterest;
            j["OpenPrice"] = ori.OpenPrice;
            j["HighestPrice"] = ori.HighestPrice;
            j["LowestPrice"] = ori.LowestPrice;
            j["Volume"] = ori.Volume;
            j["Turnover"] = ori.Turnover;
            j["OpenInterest"] = ori.OpenInterest;
            j["ClosePrice"] = ori.ClosePrice;
            j["SettlementPrice"] = ori.SettlementPrice;
            j["UpperLimitPrice"] = ori.UpperLimitPrice;
            j["LowerLimitPrice"] = ori.LowerLimitPrice;
            j["PreDelta"] = ori.PreDelta;
            j["CurrDelta"] = ori.CurrDelta;
            j["UpdateTime"] = std::string(ori.UpdateTime);
            j["UpdateMillisec"] = ori.UpdateMillisec;
            j["BidPrice1"] = ori.BidPrice1;
            j["BidVolume1"] = ori.BidVolume1;
            j["AskPrice1"] = ori.AskPrice1;
            j["AskVolume1"] = ori.AskVolume1;
            j["BidPrice2"] = ori.BidPrice2;
            j["BidVolume2"] = ori.BidVolume2;
            j["AskPrice2"] = ori.AskPrice2;
            j["AskVolume2"] = ori.AskVolume2;
            j["BidPrice3"] = ori.BidPrice3;
            j["BidVolume3"] = ori.BidVolume3;
            j["AskPrice3"] = ori.AskPrice3;
            j["AskVolume3"] = ori.AskVolume3;
            j["BidPrice4"] = ori.BidPrice4;
            j["BidVolume4"] = ori.BidVolume4;
            j["AskPrice4"] = ori.AskPrice4;
            j["AskVolume4"] = ori.AskVolume4;
            j["BidPrice5"] = ori.BidPrice5;
            j["BidVolume5"] = ori.BidVolume5;
            j["AskPrice5"] = ori.AskPrice5;
            j["AskVolume5"] = ori.AskVolume5;
            j["AveragePrice"] = ori.AveragePrice;
            j["ActionDay"] = ori.ActionDay;
        }

        inline void to_json(nlohmann::json& j, const CThostFtdcTradeField& ori)
        {
            j["BrokerID"] = std::string(ori.BrokerID);
            j["InvestorID"] = std::string(ori.InvestorID);
            j["InstrumentID"] = std::string(ori.InstrumentID);
            j["OrderRef"] = std::string(ori.OrderRef);
            j["UserID"] = std::string(ori.UserID);
            j["ExchangeID"] = std::string(ori.ExchangeID);
            j["TradeID"] = std::string(ori.TradeID);
            j["Direction"] = std::string(1, ori.Direction);
            j["OrderSysID"] = std::string(ori.OrderSysID);
            j["ParticipantID"] = std::string(ori.ParticipantID);
            j["ClientID"] = std::string(ori.ClientID);
            j["TradingRole"] = std::string(1, ori.TradingRole);
            j["ExchangeInstID"] = std::string(ori.ExchangeInstID);
            j["OffsetFlag"] = std::string(1, ori.OffsetFlag);
            j["HedgeFlag"] = std::string(1, ori.HedgeFlag);
            j["Price"] = ori.Price;
            j["Volume"] = ori.Volume;
            j["TradeDate"] = ori.TradeDate;
            j["TradeTime"] = ori.TradeTime;
            j["TradeType"] = std::string(1, ori.TradeType);
            j["PriceSource"] = std::string(1, ori.PriceSource);
            j["TraderID"] = std::string(ori.TraderID);
            j["OrderLocalID"] = std::string(ori.OrderLocalID);
            j["ClearingPartID"] = std::string(ori.ClearingPartID);
            j["BusinessUnit"] = std::string(ori.BusinessUnit);
            j["SequenceNo"] = ori.SequenceNo;
            j["TradingDay"] = std::string(ori.TradingDay);
            j["SettlementID"] = ori.SettlementID;
            j["BrokerOrderSeq"] = ori.BrokerOrderSeq;
            j["TradeSource"] = std::string(1, ori.TradeSource);
        }

        inline void to_json(nlohmann::json& j, const CThostFtdcInputOrderActionField& ori)
        {
            //TODO
        }

        template<typename T> std::string to_string(const T& ori)
        {
            nlohmann::json j;
            to_json(j, ori);
            return j.dump();
        }
    }
}
#endif //KUNGFU_SERIALIZE_H


