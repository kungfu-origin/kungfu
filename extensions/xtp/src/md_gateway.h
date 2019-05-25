//
// Created by qlu on 2019/2/11.
//

#ifndef MD_GATEWAY_XTP_H
#define MD_GATEWAY_XTP_H

#include <string>
#include <map>
#include <kungfu/wingchun/gateway/gateway_impl.h>

#include "xtp_quote_api.h"

namespace kungfu
{
    namespace xtp
    {
        class MdGateway: public XTP::API::QuoteSpi, public kungfu::MdGatewayImpl
        {
        public:
            MdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double);
            virtual ~MdGateway();

            virtual void init();
            virtual void start();

            virtual bool subscribe(const std::vector<Instrument>& instruments, bool is_level2 = false);
            virtual bool unsubscribe(const std::vector<Instrument>& instruments) { return false; };

            ///当客户端与行情后台通信连接断开时，该方法被调用。
            ///@param reason 错误原因，请与错误代码表对应
            ///@remark api不会自动重连，当断线发生时，请用户自行选择后续操作。可以在此函数中调用Login重新登录。注意用户重新登录后，需要重新订阅行情
            virtual void OnDisconnected(int reason);


            ///错误应答
            ///@param error_info 当服务器响应发生错误时的具体的错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 此函数只有在服务器发生错误时才会调用，一般无需用户处理
            virtual void OnError(XTPRI *error_info) {};

            ///订阅行情应答，包括股票、指数和期权
            ///@param ticker 详细的合约订阅情况
            ///@param error_info 订阅合约发生错误时的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 每条订阅的合约均对应一条订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last);

            ///退订行情应答，包括股票、指数和期权
            ///@param ticker 详细的合约取消订阅情况
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次取消订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 每条取消订阅的合约均对应一条取消订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnUnSubMarketData(XTPST *ticker, XTPRI *error_info, bool is_last) {};

            ///深度行情通知，包含买一卖一队列
            ///@param market_data 行情数据
            ///@param bid1_qty 买一队列数据
            ///@param bid1_count 买一队列的有效委托笔数
            ///@param max_bid1_count 买一队列总委托笔数
            ///@param ask1_qty 卖一队列数据
            ///@param ask1_count 卖一队列的有效委托笔数
            ///@param max_ask1_count 卖一队列总委托笔数
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnDepthMarketData(XTPMD *market_data, int64_t bid1_qty[], int32_t bid1_count, int32_t max_bid1_count, int64_t ask1_qty[], int32_t ask1_count, int32_t max_ask1_count);

            ///订阅行情订单簿应答，包括股票、指数和期权
            ///@param ticker 详细的合约订阅情况
            ///@param error_info 订阅合约发生错误时的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 每条订阅的合约均对应一条订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) {};

            ///退订行情订单簿应答，包括股票、指数和期权
            ///@param ticker 详细的合约取消订阅情况
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次取消订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 每条取消订阅的合约均对应一条取消订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnUnSubOrderBook(XTPST *ticker, XTPRI *error_info, bool is_last) {};

            ///行情订单簿通知，包括股票、指数和期权
            ///@param order_book 行情订单簿数据，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnOrderBook(XTPOB *order_book) {};

            ///订阅逐笔行情应答，包括股票、指数和期权
            ///@param ticker 详细的合约订阅情况
            ///@param error_info 订阅合约发生错误时的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 每条订阅的合约均对应一条订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last);

            ///退订逐笔行情应答，包括股票、指数和期权
            ///@param ticker 详细的合约取消订阅情况
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次取消订阅的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 每条取消订阅的合约均对应一条取消订阅应答，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnUnSubTickByTick(XTPST *ticker, XTPRI *error_info, bool is_last) {};

            ///逐笔行情通知，包括股票、指数和期权
            ///@param tbt_data 逐笔行情数据，包括逐笔委托和逐笔成交，此为共用结构体，需要根据type来区分是逐笔委托还是逐笔成交，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnTickByTick(XTPTBT *tbt_data);

            ///订阅全市场的股票行情应答
            ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///退订全市场的股票行情应答
            ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnUnSubscribeAllMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///订阅全市场的股票行情订单簿应答
            ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///退订全市场的股票行情订单簿应答
            ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnUnSubscribeAllOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///订阅全市场的股票逐笔行情应答
            ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///退订全市场的股票逐笔行情应答
            ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnUnSubscribeAllTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};


            ///查询可交易合约的应答
            ///@param ticker_info 可交易合约信息
            ///@param error_info 查询可交易合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次查询可交易合约的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            virtual void OnQueryAllTickers(XTPQSI* ticker_info, XTPRI *error_info, bool is_last) {};

            ///查询合约的最新价格信息应答
            ///@param ticker_info 合约的最新价格信息
            ///@param error_info 查询合约的最新价格信息时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param is_last 是否此次查询的最后一个应答，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            virtual void OnQueryTickersPriceInfo(XTPTPI* ticker_info, XTPRI *error_info, bool is_last) {};

            ///订阅全市场的期权行情应答
            ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///退订全市场的期权行情应答
            ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnUnSubscribeAllOptionMarketData(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///订阅全市场的期权行情订单簿应答
            ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///退订全市场的期权行情订单簿应答
            ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnUnSubscribeAllOptionOrderBook(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///订阅全市场的期权逐笔行情应答
            ///@param exchange_id 表示当前全订阅的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

            ///退订全市场的期权逐笔行情应答
            ///@param exchange_id 表示当前退订的市场，如果为XTP_EXCHANGE_UNKNOWN，表示沪深全市场，XTP_EXCHANGE_SH表示为上海全市场，XTP_EXCHANGE_SZ表示为深圳全市场
            ///@param error_info 取消订阅合约时发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 需要快速返回
            virtual void OnUnSubscribeAllOptionTickByTick(XTP_EXCHANGE_TYPE exchange_id, XTPRI *error_info) {};

        private:
            int client_id_;
            std::string save_file_path_;
            std::string ip_;
            int port_;
            std::string user_;
            std::string password_;

            XTP::API::QuoteApi* api_;

            bool login();
            bool subscribe(const std::vector<std::string> &instruments, const std::string& exchange_id, bool is_level2);
        };
    }
}

#endif //MD_GATEWAY_XTP_H
