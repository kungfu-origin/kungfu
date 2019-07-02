export const quote = 101
export const entrust = 102
export const transaction = 103

export const orderInput = 201
export const orderAction = 202
export const order = 203
export const trade = 204
export const position = 205  //监听持仓
export const accountInfo = 206 //监听资金
export const portfolio = 207 //监听收益曲线分钟线
export const accountInfoByMin = 208 //账户信息分钟线，仅td推送
export const portfolioByMin = 209 //投资组合信息分钟线，仅策略推送


export const reqLogin = 301
export const subscribe = 302

export const makeOrder = 353
export const cancelOrder = 354

export const gatewayState = 401

export const calendar = 601  //自动推送日期相关信息，目前只有交易日
export const reqCalendar = 602 //主动日期相关信息，目前只有交易日

export const strategyUsedAccountUpdate = 701 //策略中账户的更新


export const category = {
    0: 'md',
    1: 'td',
    2: 'strategy'
}