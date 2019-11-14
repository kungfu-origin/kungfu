| 报单类型 |  上交所  |  深交所  | 上期所 |  中金所  |  大商所  |  郑商所  |
| :----: | :----: | :----: | :----: | :----: | :----: | :----: |
| Limit |限价|限价|限价|限价|限价|限价|
| Any |最优五档剩余撤销|即时成交剩余撤销|市价|市价|市价|市价|
| FakBest5 |最优五档即时成交剩余撤销|最优五档即时成交剩余撤销|不支持|不支持|不支持|不支持|
| ForwardBest |最优五档即时成交剩余转限价|对手方最优价格申报|不支持|不支持|不支持|不支持|
| ReverseBest |本方最优价格申报|不支持|不支持|不支持|不支持|不支持|
| Fak |不支持|(市价)即时成交剩余撤销|(限价)即时成交剩余撤销|(限价)即时成交剩余撤销|(限价)即时成交剩余撤销|(限价)即时成交剩余撤销|
| Fok |不支持|(市价)全额成交或者撤销|(限价)全额成交或者撤销|(限价)全额成交或者撤销|(限价)全额成交或者撤销|(限价)全额成交或者撤销|

期货fok与fak
报单价格为限价，报单时间和交易量条件如下

| 报单类型 | PriceType  |  TimeCondition  |  VolumeCondition  |
| :----: | :----: | :----: | :----: |
| Fok |Limit|IOC|All|
| Fak |Limit|IOC|Any|

order_id = context.insert_order(ticker, exchange, account, price, volume, type , side, offset = Offset.Open, hedge_flag = HedgeFlag.Speculation)

