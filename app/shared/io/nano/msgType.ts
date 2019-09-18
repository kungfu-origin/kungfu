export const order = 203
export const trade = 204
export const position = 205  //监听持仓
export const accountInfo = 206 //监听资金
export const portfolio = 207 //监听收益曲线分钟线

export const makeOrder = 353
export const cancelOrder = 354
export const cancelAllOrder = 355

export const reqGatewayState = 400
export const gatewayState = 401
export const reqCash = 403
export const reqRemoveStrategyPos = 404

export const calendar = 601  //自动推送日期相关信息，目前只有交易日

export const category: StringToStringObject = {
    '0': 'md',
    '1': 'td',
    '2': 'strategy'
}