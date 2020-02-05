//
// Created by Keren Dong on 2020/1/28.
//

#ifndef KUNGFU_LONGFIST_ENUM_H
#define KUNGFU_LONGFIST_ENUM_H

#include <fmt/ostream.h>
#include <spdlog/fmt/ostr.h>

namespace kungfu::longfist
{

    enum class InstrumentType : int8_t
    {
        Unknown,       //未知
        Stock,         //普通股票
        Future,        //期货
        Bond,          //债券
        StockOption,   //股票期权
        Fund,          //基金
        TechStock,     //科创板股票
        Index,         //指数
        Repo           //回购
    };

    inline std::ostream& operator<<( std::ostream& os, InstrumentType t )
    {
        return os << (int8_t) t;
    }

    enum class ExecType : int8_t
    {
        Unknown,
        Cancel,
        Trade
    };

    inline std::ostream& operator<<( std::ostream& os, ExecType t )
    {
        return os << (int8_t) t;
    }

    enum class BsFlag : int8_t
    {
        Unknown,
        Buy,
        Sell
    };

    inline std::ostream& operator<<( std::ostream& os, BsFlag t )
    {
        return os << (int8_t) t;
    }

    enum class Side : int8_t
    {
        Buy,
        Sell,
        Lock,
        Unlock,
        Exec,
        Drop
    };

    inline std::ostream& operator<<( std::ostream& os, Side t )
    {
        return os << (int8_t) t;
    }

    enum class Offset : int8_t
    {
        Open,
        Close,
        CloseToday,
        CloseYesterday
    };

    inline std::ostream& operator<<( std::ostream& os, Offset t )
    {
        return os << (int8_t) t;
    }

    enum class HedgeFlag : int8_t
    {
        Speculation,
        Arbitrage,
        Hedge,
        Covered
    };

    inline std::ostream& operator<<( std::ostream& os, HedgeFlag t )
    {
        return os << (int8_t) t;
    }

    enum class OrderActionFlag : int8_t
    {
        Cancel,
    };

    inline std::ostream& operator<<( std::ostream& os, OrderActionFlag t )
    {
        return os << (int8_t) t;
    }

    enum class PriceType : int8_t
    {
        Limit,                  //限价,证券通用
        Any,                    //市价，证券通用，对于股票上海为最优五档剩余撤销，深圳为即时成交剩余撤销，建议客户采用
        FakBest5,               //上海深圳最优五档即时成交剩余撤销，不需要报价
        ForwardBest,            //深圳本方方最优价格申报, 不需要报价
        ReverseBest,            //上海最优五档即时成交剩余转限价, 深圳对手方最优价格申报，不需要报价
        Fak,                    //深圳即时成交剩余撤销，不需要报价
        Fok,                    //深圳市价全额成交或者撤销，不需要报价
        UnKnown
    };

    inline std::ostream& operator<<( std::ostream& os, PriceType t )
    {
        return os << (int8_t) t;
    }

    enum class VolumeCondition : int8_t
    {
        Any,
        Min,
        All
    };

    inline std::ostream& operator<<( std::ostream& os, VolumeCondition t )
    {
        return os << (int8_t) t;
    }

    enum class TimeCondition : int8_t
    {
        IOC,
        GFD,
        GTC
    };

    inline std::ostream& operator<<( std::ostream& os, TimeCondition t )
    {
        return os << (int8_t) t;
    }

    enum class OrderStatus : int8_t
    {
        Unknown,
        Submitted,
        Pending,
        Cancelled,
        Error,
        Filled,
        PartialFilledNotActive,
        PartialFilledActive
    };

    inline std::ostream& operator<<( std::ostream& os, OrderStatus t )
    {
        return os << (int8_t) t;
    }

    enum class Direction : int8_t
    {
        Long,
        Short
    };

    inline std::ostream& operator<<( std::ostream& os, Direction t )
    {
        return os << (int8_t) t;
    }

    enum class AccountType : int8_t
    {
        Stock,
        Credit,
        Future
    };

    inline std::ostream& operator<<( std::ostream& os, AccountType t )
    {
        return os << (int8_t) t;
    }

    enum class CommissionRateMode : int8_t
    {
        ByAmount,
        ByVolume
    };

    inline std::ostream& operator<<( std::ostream& os, CommissionRateMode t )
    {
        return os << (int8_t) t;
    }

    enum class LedgerCategory : int8_t
    {
        Account,
        Strategy,
    };

    inline std::ostream& operator<<( std::ostream& os, LedgerCategory t )
    {
        return os << (int8_t) t;
    }

    enum class BrokerState : int8_t
    {
        Unknown = 0,
        Idle = 1,
        DisConnected = 2,
        Connected = 3,
        LoggedIn = 4,
        LoggedInFailed = 5,
        Ready = 100
    };

    inline std::ostream& operator<<( std::ostream& os, BrokerState t )
    {
        return os << (int8_t) t;
    }
}
#endif //KUNGFU_LONGFIST_ENUM_H
