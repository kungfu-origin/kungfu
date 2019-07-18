//
// Created by qlu on 2019/2/11.
//

#ifndef MD_GATEWAY_XTP_H
#define MD_GATEWAY_XTP_H

#include <string>
#include <map>

#include <kungfu/yijinjing/common.h>
#include <kungfu/wingchun/msg.h>
#include <kungfu/wingchun/gateway/marketdata.h>

#include "xtp_quote_api.h"

namespace kungfu
{
    namespace wingchun
    {
        namespace xtp
        {
            class MdGateway : public XTP::API::QuoteSpi, public gateway::MarketData
            {
            public:
                MdGateway(bool low_latency, yijinjing::data::locator_ptr locator, std::map<std::string, std::string> &config_str, std::map<std::string, int> &config_int,
                          std::map<std::string, double> &config_double);

                ~MdGateway() override;

                bool subscribe(const std::vector<wingchun::msg::data::Instrument> &instruments) override;

                bool unsubscribe(const std::vector<wingchun::msg::data::Instrument> &instruments) override
                { return false; };

                ///当客户端与行情后台通信连接断开时，该方法被调用。
                ///@param reason 错误原因，请与错误代码表对应
                ///@remark api不会自动重连，当断线发生时，请用户自行选择后续操作。可以在此函数中调用Login重新登录。注意用户重新登录后，需要重新订阅行情
                void OnDisconnected(int reason) override;


                ///错误应答
                ///@param error_info 当服务器响应发生错误时的具体的错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 此函数只有在服务器发生错误时才会调用，一般无需用户处理
                void OnError(XTPRI *error_info) override
                {};

                ///订阅行情应答，包括股票、指数和期权
                ///@param ticker 详细的合约订阅情况
                ///@param error_info 订阅合约发生错误时的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                ///@remark 每条订阅的合约均对应一条订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) override;

                ///退订行情应答，包括股票、指数和期权
                ///@param ticker 详细的合约取消订阅情况
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次取消订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                ///@remark 每条取消订阅的合约均对应一条取消订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) override
                {};

                ///深度行情通知，包含买一卖一队列
                ///@param market_data 行情数据
                ///@param bid1_qty 买一队列数据
                ///@param bid1_count 买一队列的有效委托笔数
                ///@param max_bid1_count 买一队列总委托笔数
                ///@param ask1_qty 卖一队列数据
                ///@param ask1_count 卖一队列的有效委托笔数
                ///@param max_ask1_count 卖一队列总委托笔数
                ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[],
                                       int32_t ask1_count, int32_t max_ask1_count) override;

                ///订阅行情订单簿应答，包括股票、指数和期权
                ///@param ticker 详细的合约订阅情况
                ///@param error_info 订阅合约发生错误时的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                ///@remark 每条订阅的合约均对应一条订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) override
                {};

                ///退订行情订单簿应答，包括股票、指数和期权
                ///@param ticker 详细的合约取消订阅情况
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次取消订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                ///@remark 每条取消订阅的合约均对应一条取消订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) override
                {};

                ///行情订单簿通知，包括股票、指数和期权
                ///@param order_book 行情订单簿数据，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnOrderBook(XTPOB *order_book) override
                {};

                ///订阅逐笔行情应答，包括股票、指数和期权
                ///@param ticker 详细的合约订阅情况
                ///@param error_info 订阅合约发生错误时的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                ///@remark 每条订阅的合约均对应一条订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) override;

                ///退订逐笔行情应答，包括股票、指数和期权
                ///@param ticker 详细的合约取消订阅情况
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次取消订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                ///@remark 每条取消订阅的合约均对应一条取消订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) override
                {};

                ///逐笔行情通知，包括股票、指数和期权
                ///@param tbt_data 逐笔行情数据，包括逐笔委托和逐笔成交，此为共用结构体，需要根据type来区分是逐笔委托还是逐笔成交，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
                void OnTickByTick(XTPTBT *tbt_data) override;

                ///订阅全市场的股票行情应答
                ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///退订全市场的股票行情应答
                ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///订阅全市场的股票行情订单簿应答
                ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///退订全市场的股票行情订单簿应答
                ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///订阅全市场的股票逐笔行情应答
                ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///退订全市场的股票逐笔行情应答
                ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};


                ///查询可交易合约的应答
                ///@param ticker_info 可交易合约信息
                ///@param error_info 查询可交易合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次查询可交易合约的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                void OnQueryAllTickers(XTPQSI *ticker_info, XTPRI *error_info, bool is_last) override
                {};

                ///查询合约的最新价格信息应答
                ///@param ticker_info 合约的最新价格信息
                ///@param error_info 查询合约的最新价格信息时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@param is_last 是否此次查询的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
                void OnQueryTickersPriceInfo(XTPTPI *ticker_info, XTPRI *error_info, bool is_last) override
                {};

                ///订阅全市场的期权行情应答
                ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///退订全市场的期权行情应答
                ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnUnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///订阅全市场的期权行情订单簿应答
                ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///退订全市场的期权行情订单簿应答
                ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnUnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///订阅全市场的期权逐笔行情应答
                ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

                ///退订全市场的期权逐笔行情应答
                ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
                ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
                ///@remark 需要快速返回
                void OnUnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) override
                {};

            protected:

                void on_start() override;

            private:
                int client_id_;
                std::string save_file_path_;
                std::string ip_;
                int port_;
                std::string user_;
                std::string password_;

                XTP::API::QuoteApi *api_;

                bool subscribe(const std::vector<std::string> &instruments, const std::string &exchange_id);
            };
        }
    }
}

#endif //MD_GATEWAY_XTP_H
