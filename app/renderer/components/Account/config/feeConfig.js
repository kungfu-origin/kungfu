import {blankValidator, validateTCPUri, validate099, validateInt} from '@/assets/js/validator'
/**
 * @param  {String} type stock/future
 */
export const feeTemplate = (type) => {
    return {
        instrument_id: {
            label: '代码',
            type: 'string',
            validate: []
        },
        exchange_id: {
            label: '交易所代码',
            type: 'string',
        },
        instrument_type: {
            label: '类型',
            type: 'select',
            options: {
                0: '未知',
                1: '股票',
                2: '期货',
                3: '债券',
                4: '股票期权'
            }
        },
        account_id: {
            label: "账号",
            type: 'string',
        },
        mode: {
            label: '模式',
            type: 'select',
            options: {
                0: '按成交金额',
                1: '按成交量',
            }
        },
        open_ratio: {
            label: type === 'stock' ? '买入费率' : '开仓费率',
            type: 'number',
        },
        close_ratio: {
            label: type === 'stock' ? '卖出费率' : '平仓费率',
            type: 'number',
        },
        ...((type === 'future') ? {close_today_ratio : {
            label: '平今费率',
            type: 'number',
        }}: {}),
        min_commission: {
            label: '最小手续费',
            type: 'number'
        }
    }

}