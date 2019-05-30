//
// Created by qlu on 2019/1/11.
//

#ifndef KUNGFU_DATA_TYPE_H
#define KUNGFU_DATA_TYPE_H

#include <cstdint>
#include <kungfu/wingchun/config.h>

namespace kungfu
{
    #define REGION_CN "CN"
    #define REGION_HK "HK"

    #define EXCHANGE_SSE "SSE"
    #define EXCHANGE_SZE "SZE"
    #define EXCHANGE_SHFE "SHFE"
    #define EXCHANGE_DCE "DCE"
    #define EXCHANGE_CZCE "CZCE"
    #define EXCHANGE_CFFEX "CFFEX"
    #define EXCHANGE_INE "INE"

    const int INSTRUMENT_ID_LEN = 32;
    const int PRODUCT_ID_LEN = 32;
    const int DATE_LEN = 9;
    const int EXCHANGE_ID_LEN = 16;
    const int ACCOUNT_ID_LEN = 32;
    const int CLIENT_ID_LEN = 32;
    const int EXEC_ID_LEN = 32;
    const int SOURCE_ID_LEN = 16;
    const int BROKER_ID_LEN = 32;
    const int ERROR_MSG_LEN = 32;

    typedef char InstrumentType;
    const InstrumentType InstrumentTypeUnknown = '0';
    const InstrumentType InstrumentTypeStock = '1';
    const InstrumentType InstrumentTypeFuture = '2';
    const InstrumentType InstrumentTypeBond = '3';
    const InstrumentType InstrumentTypeStockOption = '4';

    typedef char ExecType;
    const ExecType ExecTypeUnknown = '0';
    const ExecType ExecTypeCancel = '1';
    const ExecType ExecTypeTrade = '2';

    typedef char BsFlag;
    const BsFlag BsFlagUnknown = '0';
    const BsFlag BsFlagBuy = '1';
    const BsFlag  BsFlagSell = '2';

    typedef char Side;
    const Side SideBuy = '0'; // 所有建仓操作(股票买入, 逆回购借出)
    const Side SideSell= '1';

    typedef char Offset;
    const Offset OffsetOpen = '0';
    const Offset OffsetClose = '1';
    const Offset OffsetCloseToday = '2';
    const Offset OffsetCloseYesterday = '3';

    typedef char OrderActionFlag;
    const OrderActionFlag OrderActionFlagCancel = '0';

    typedef char PriceType;
    const PriceType PriceTypeAny = '0';
    const PriceType PriceTypeBest = '1';
    const PriceType PriceTypeBest5 = '2';
    const PriceType PriceTypeLimit = '3';
    const PriceType PriceTypeForwardBest = '4';
    const PriceType PriceTypeReverseBest = '5';

    typedef char VolumeCondition;
    const VolumeCondition VolumeConditionAny = '0';
    const VolumeCondition VolumeConditionMin = '1';
    const VolumeCondition VolumeConditionAll = '2';

    typedef char TimeCondition;
    const TimeCondition TimeConditionIOC = '0';
    const TimeCondition TimeConditionGFD = '1';
    const TimeCondition TimeConditionGTC = '2';

    typedef char OrderStatus;
    const OrderStatus OrderStatusUnknown = '0';
    const OrderStatus OrderStatusSubmitted = '1';
    const OrderStatus OrderStatusPending = '2';
    const OrderStatus OrderStatusCancelled = '3';
    const OrderStatus OrderStatusError = '4';
    const OrderStatus OrderStatusFilled = '5';
    const OrderStatus OrderStatusPartialFilledNotActive = '6';
    const OrderStatus OrderStatusPartialFilledActive = '7';

    typedef char Direction;
    const Direction DirectionLong = '0';
    const Direction DirectionShort = '1';

    typedef char AccountType;
    const AccountType AccountTypeStock = '0';
    const AccountType AccountTypeCredit = '1';
    const AccountType AccountTypeFuture = '2';

    typedef char CommissionRateMode;
    const CommissionRateMode CommissionRateModeByAmount = '0';
    const CommissionRateMode CommissionRateModeByVolume = '1';

}

#endif //KUNGFU_DATA_TYPE_H
