//
// Created by qlu on 2019/1/23.
//

#ifndef PROJECT_MSG_H
#define PROJECT_MSG_H

namespace kungfu
{
    enum class MsgType: int
    {
        Error = 0,

        Quote = 101,
        Entrust = 102,
        Transaction = 103,

        OrderInput = 201,
        OrderAction = 202,
        Order = 203,
        Trade = 204,
        Position = 205,
        AccountInfo = 206,
        Portfolio = 207,
        AccountInfoByMin = 208,
        PortfolioByMin = 209,
        PositionDetail = 210,
        SubPortfolioInfo = 211,


        ReqLogin = 301,
        Subscribe = 302,
        ReqOrderInput = 303,
        ReqOrderAction = 304,

        RspLogin = 351,
        RspSubscribe = 352,
        RspOrderInput = 353,
        RspOrderAction = 354,

        GatewayState = 401,

        AlgoOrderInput = 501,
        AlgoOrderStatus = 502,
        AlgoOrderAction = 503,

        SwitchDay = 601,
        RspTradingDay = 602,

        ReloadFutureInstrument = 701
    };
}

#endif //PROJECT_MSG_H
