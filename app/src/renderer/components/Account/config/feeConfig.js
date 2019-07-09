import {blankValidator, validateTCPUri, validate099, validateInt} from '__assets/validator'
/**
 * @param  {String} type stock/future
 */
export const feeTemplate = (type) => {
    return {
        instrument_id: {
            label: type === 'future' ? '代码/品种' : '代码',
            type: 'string',
            validate: []
        },
        instrument_type: {
            label: '类型',
            type: 'select',
            options: {
                1: '股票',
                2: '期货',
                3: '债券',
                4: '股票期权'
            }
        },
        mode: {
            label: '模式',
            type: 'select',
            options: {
                0: '成交金额',
                1: '成交量',
            }
        },
        open_ratio: {
            label: type === 'stock' ? '买入费率' : '开仓费率',
            type: 'number',
            ratio: true,
        },
        close_ratio: {
            label: type === 'stock' ? '卖出费率' : '平仓费率',
            type: 'number',
            ratio: true,
        },
        ...((type === 'future') ? {close_today_ratio : {
            label: '平今费率',
            type: 'number',
            ratio: true,
        }}: {}),
        min_commission: {
            label: '最小值',
            type: 'number',
            ratio: false,
        }
    }

}