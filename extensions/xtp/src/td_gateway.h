//
// Created by qlu on 2019/2/11.
//

#ifndef TD_GATEWAY_XTP_H
#define TD_GATEWAY_XTP_H

#include <memory>
#include <string>
#include <map>

#include <kungfu/wingchun/gateway/gateway_impl.h>

#include "xtp_trader_api.h"
#include "order_mapper.h"

namespace kungfu
{
    namespace xtp
    {
        class TdGateway: public XTP::API::TraderSpi, public kungfu::TdGatewayImpl
        {
        public:
            TdGateway(std::map<std::string, std::string>& config_str, std::map<std::string, int>& config_int, std::map<std::string, double>& config_double);
            virtual ~TdGateway();

            virtual void init();
            virtual void start();
            virtual const std::string& get_account_id() const { return user_; }
            virtual const AccountType get_account_type() const { return AccountTypeStock; }

            virtual bool insert_order(const OrderInput& input);
            virtual bool cancel_order(const OrderAction& action);
            virtual bool req_position();
            virtual bool req_account();

            bool login();

            ///当客户端的某个连接与交易后台通信连接断开时，该方法被调用。
            ///@param reason 错误原因，请与错误代码表对应
            ///@param session_id 资金账户对应的session_id，登录时得到
            ///@remark 用户主动调用logout导致的断线，不会触发此函数。api不会自动重连，当断线发生时，请用户自行选择后续操作，可以在此函数中调用Login重新登录，并更新session_id，此时用户收到的数据跟断线之前是连续的
            virtual void OnDisconnected(uint64_t session_id, int reason);

            ///错误应答
            ///@param error_info 当服务器响应发生错误时的具体的错误代码和错误信息,当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 此函数只有在服务器发生错误时才会调用，一般无需用户处理
            virtual void OnError(XTPRI *error_info) {};

            ///报单通知
            ///@param order_info 订单响应具体信息，用户可以通过order_info.order_xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单，order_info.qty_left字段在订单为未成交、部成、全成、废单状态时，表示此订单还没有成交的数量，在部撤、全撤状态时，表示此订单被撤的数量。order_info.order_cancel_xtp_id为其所对应的撤单ID，不为0时表示此单被撤成功
            ///@param error_info 订单被拒绝或者发生错误时错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 每次订单状态更新时，都会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线，在订单未成交、全部成交、全部撤单、部分撤单、已拒绝这些状态时会有响应，对于部分成交的情况，请由订单的成交回报来自行确认。所有登录了此用户的客户端都将收到此用户的订单响应
            virtual void OnOrderEvent(XTPOrderInfo *order_info, XTPRI *error_info, uint64_t session_id);

            ///成交通知
            ///@param trade_info 成交回报的具体信息，用户可以通过trade_info.order_xtp_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。对于上交所，exec_id可以唯一标识一笔成交。当发现2笔成交回报拥有相同的exec_id，则可以认为此笔交易自成交了。对于深交所，exec_id是唯一的，暂时无此判断机制。report_index+market字段可以组成唯一标识表示成交回报。
            ///@remark 订单有成交发生的时候，会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。所有登录了此用户的客户端都将收到此用户的成交回报。相关订单为部成状态，需要用户通过成交回报的成交数量来确定，OnOrderEvent()不会推送部成状态。
            virtual void OnTradeEvent(XTPTradeReport *trade_info, uint64_t session_id);

            ///撤单出错响应
            ///@param cancel_info 撤单具体信息，包括撤单的order_cancel_xtp_id和待撤单的order_xtp_id
            ///@param error_info 撤单被拒绝或者发生错误时错误代码和错误信息，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 此响应只会在撤单发生错误时被回调
            virtual void OnCancelOrderError(XTPOrderCancelInfo *cancel_info, XTPRI *error_info, uint64_t session_id);

            ///请求查询报单响应
            ///@param order_info 查询到的一个报单
            ///@param error_info 查询报单时发生错误时，返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryOrder(XTPQueryOrderRsp *order_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

            ///请求查询成交响应
            ///@param trade_info 查询到的一个成交回报
            ///@param error_info 查询成交回报发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 由于支持分时段查询，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryTrade(XTPQueryTradeRsp *trade_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

            ///请求查询投资者持仓响应
            ///@param position 查询到的一只股票的持仓情况
            ///@param error_info 查询账户持仓发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 由于用户可能持有多个股票，一个查询请求可能对应多个响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryPosition(XTPQueryStkPositionRsp *position, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

            ///请求查询资金账户响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            ///@param asset 查询到的资金账户情况
            ///@param error_info 查询资金账户发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryAsset(XTPQueryAssetRsp *asset, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id);

            ///请求查询分级基金信息响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            ///@param fund_info 查询到的分级基金情况
            ///@param error_info 查询分级基金发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryStructuredFund(XTPStructuredFundInfo *fund_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

            ///请求查询资金划拨订单响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            ///@param fund_transfer_info 查询到的资金账户情况
            ///@param error_info 查询资金账户发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryFundTransfer(XTPFundTransferNotice *fund_transfer_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

            ///资金划拨通知
            ///@param fund_transfer_info 资金划拨通知的具体信息，用户可以通过fund_transfer_info.serial_id来管理订单，通过GetClientIDByXTPID() == client_id来过滤自己的订单。
            ///@param error_info 资金划拨订单被拒绝或者发生错误时错误代码和错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@remark 当资金划拨订单有状态变化的时候，会被调用，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线。所有登录了此用户的客户端都将收到此用户的资金划拨通知。
            virtual void OnFundTransfer(XTPFundTransferNotice *fund_transfer_info, XTPRI *error_info, uint64_t session_id) {};

            ///请求查询ETF清单文件的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            ///@param etf_info 查询到的ETF清单文件情况
            ///@param error_info 查询ETF清单文件发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryETF(XTPQueryETFBaseRsp *etf_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

            ///请求查询ETF股票篮的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            ///@param etf_component_info 查询到的ETF合约的相关成分股信息
            ///@param error_info 查询ETF股票篮发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryETFBasket(XTPQueryETFComponentRsp *etf_component_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

            ///请求查询今日新股申购信息列表的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            ///@param ipo_info 查询到的今日新股申购的一只股票信息
            ///@param error_info 查询今日新股申购信息列表发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryIPOInfoList(XTPQueryIPOTickerRsp *ipo_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

            ///请求查询用户新股申购额度信息的响应，需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            ///@param quota_info 查询到的用户某个市场的今日新股申购额度信息
            ///@param error_info 查查询用户新股申购额度信息发生错误时返回的错误信息，当error_info为空，或者error_info.error_id为0时，表明没有错误
            ///@param request_id 此消息响应函数对应的请求ID
            ///@param is_last 此消息响应函数是否为request_id这条请求所对应的最后一个响应，当为最后一个的时候为true，如果为false，表示还有其他后续消息响应
            ///@remark 需要快速返回，否则会堵塞后续消息，当堵塞严重时，会触发断线
            virtual void OnQueryIPOQuotaInfo(XTPQueryIPOQuotaRsp *quota_info, XTPRI *error_info, int request_id, bool is_last, uint64_t session_id) {};

        private:
            int client_id_;
            std::string software_key_;
            std::string save_file_path_;
            std::string ip_;
            int port_;
            std::string user_;
            std::string password_;

            XTP::API::TraderApi* api_;
            uint64_t session_id_;

            int request_id_;

            std::shared_ptr<OrderMapper> order_mapper_;

        };
    }
}
#endif //TD_GATEWAY_XTP_H
