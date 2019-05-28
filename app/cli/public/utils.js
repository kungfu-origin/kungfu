String.prototype.toAccountId = function(){
    return this.split('_').slice(1).join('_')
}




// open = '0',
// close = '1',
// close_today = '2',
// close_yesterday = '3'
export const offsetName = {
    '0': 'open',
    '1': 'close',
    '2': 'closeT',
    '3': 'closeY'
}

export const sideName = {
    '0': 'buy',
    '1': 'sell'
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
export const orderStatus = {
    '0': 'wait',
    '1': 'Submitted',
    '2': 'Pending',
    '3': 'Cancelled',
    '4': 'Error',
    '5': 'Filled',
    '6': 'ParticalFilled',
    '7': 'trading',
}

//     DirectionLong = '0'
//    DirectionShort = '1'
export const posDirection = {
    0: 'L',
    1: 'S' 
}
