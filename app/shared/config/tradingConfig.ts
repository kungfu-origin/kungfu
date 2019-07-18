

// open = '0',
// close = '1',
// close_today = '2',
// close_yesterday = '3'
export const offsetName: NumberToStringObject = {
    0: '开仓',
    1: '平仓',
    2: '平今',
    3: '平昨'
}

export const sideName: NumberToStringObject = {
    0: '买',
    1: '卖'
}

export const statusName: StringToStringObject = {
    all_traded: '全部成交',
    pending: '非最终状态',
    error: '错误',
    canceled: '全部撤销',
    part_traded_not_queueing: '部分撤销部分成交'
}
// Unknown = '0', // 未知
// Submitted = '1', //已提交 
// Pending = '2', // 等待
// Cancelled = '3', // 已撤销
// Error = '4', // 错误
// Filled = '5', //已成交
// PartialFilledNotActive = '6', //部分撤单
// PartialFilledActive = '7' //正在交易
// 3,4,5,6已完成
export const orderStatus: NumberToStringObject = {
    0: '等待中',
    1: '已提交',
    2: '等待中',
    3: '已撤单',
    4: '错误',
    5: '已成交',
    6: '部分撤单',
    7: '正在交易',
}

//     DirectionLong = '0'
//    DirectionShort = '1'
export const posDirection: NumberToStringObject = {
    0: '多',
    1: '空' 
}
