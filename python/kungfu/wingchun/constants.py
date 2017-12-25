# -*- coding: utf-8 -*-
'''
Copyright [2017] [taurus.ai]

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
'''

class SOURCE:
    CTP = 1
    XTP = 15

class EXCHANGE:
    SSE = 'SSE' #上海证券交易所
    SZE = 'SZE' #深圳证券交易所
    CFFEX = 'CFFEX' #中国金融期货交易所
    SHFE = 'SHFE' #上海期货交易所
    DCE = 'DCE' #大连商品交易所
    CZCE = 'CZCE' #郑州商品交易所

###################################
# LfDirectionType: 买卖方向
###################################
class DIRECTION:
    Buy = '0' #买
    Sell = '1' #卖

###################################
# LfOffsetFlagType: 开平标志
###################################
class OFFSET:
    Open = '0' #开仓
    Close = '1' #平仓
    CloseToday = '3' #平今
    CloseYesterday = '4' #平昨

###################################
# LfOrderPriceTypeType: 报单价格条件
###################################
class PRICE_TYPE:
    AnyPrice = '1' #任意价
    LimitPrice = '2' #限价
    BestPrice = '3' #最优价

###################################
# LfOrderStatusType: 报单状态
###################################
class ORDER_STATUS:
    AllTraded = '0' #全部成交（最终状态）
    PartTradedQueueing = '1' #部分成交还在队列中
    PartTradedNotQueueing = '2' #部分成交不在队列中（部成部撤， 最终状态）
    NoTradeQueueing = '3' #未成交还在队列中
    NoTradeNotQueueing = '4' #未成交不在队列中（被拒绝，最终状态）
    Canceled = '5' #撤单
    AcceptedNoReply = '6' #订单已报入交易所未应答
    Unknown = 'a' #未知
    NotTouched = 'b' #尚未触发
    Touched = 'c' #已触发
    Error = 'd' #废单错误（最终状态）
    OrderInserted = 'i' #订单已写入
    OrderAccepted = 'j' #前置已接受

###################################
# LfPosiDirectionType: 持仓多空方向
###################################
class POS_DIRECTION:
    Net = '1' #净
    Long = '2' #多头
    Short = '3' #空头

###################################
# LfVolumeConditionType: 成交量类型
###################################
class VOLUME_TYPE:
    AV = '1' #任何数量
    MV = '2' #最小数量
    CV = '3' #全部数量

###################################
# LfTimeConditionType: 有效期类型
###################################
class TIME_CONDITION:
    IOC = '1' #立即完成，否则撤销
    GFS = '2' #本节有效
    GFD = '3' #当日有效
    GTD = '4' #指定日期前有效
    GTC = '5' #撤销前有效
    GFA = '6' #集合竞价有效
