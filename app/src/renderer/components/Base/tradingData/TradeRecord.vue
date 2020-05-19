<template>
<tr-dashboard :title="name ? name : `成交记录 ${currentTitle}`">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest && !dateForHistory">
            <i class="el-icon-date mouse-over" title="历史" @click="dateRangeDialogVisiblityForHistory = true"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest && dateForHistory">
            <span>{{ dateForHistory }}</span>
            <i class="el-icon-close mouse-over" @click="handleClearHistory"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest">
            <i class="el-icon-download mouse-over" title="导出" @click="dateRangeDialogVisiblityForExport = true"></i>
        </tr-dashboard-header-item>
         <tr-dashboard-header-item v-if="!value">
            <i class="el-icon-monitor mouse-over" title="打开监控" @click="handleMonitTrades"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-else>
            <i class="el-icon-s-platform mouse-over" title="关闭监控" @click="handleMonitTrades"></i>
        </tr-dashboard-header-item>
    </div>
    <tr-table
        v-if="rendererTable"
        :data="tableData"
        :schema="schema"
        :renderCellClass="renderCellClass"
    ></tr-table>
    <date-picker-dialog 
    @confirm="handleConfirmDateRangeForExport"
    v-if="dateRangeDialogVisiblityForExport"
    :visible.sync="dateRangeDialogVisiblityForExport"   
    :loading="dateRangeExportLoading" 
    ></date-picker-dialog>
    <date-picker-dialog 
    @confirm="handleConfirmDateRangeForHistory"
    v-if="dateRangeDialogVisiblityForHistory"
    :visible.sync="dateRangeDialogVisiblityForHistory"   
    :loading="dateRangeExportLoading" 
    ></date-picker-dialog>
</tr-dashboard>

</template>

<script>

import DatePickerDialog from '../DatePickerDialog';
import tradingDataMixin from './js/tradingDataMixin';

import { debounce } from "__gUtils/busiUtils";
import { dealTrade } from "__io/kungfu/watcher";
import { writeCSV } from '__gUtils/fileUtils';

export default {
    name: 'trades-record',

    mixins: [ tradingDataMixin ],

    props: {
        orderStat: {
            type: Object,
            default: () => ({})
        },

        name: {
            type: String,
            default: ''
        }
    },

    components: {
        DatePickerDialog
    },

    data () {
        return {
            kungfuBoardType: 'trade',
        }
    },

    computed:{
        schema(){
            
            if (this.dateForHistory) {
                return [{
                    type: 'text',
                    label: '成交时间',
                    prop: "updateTimeMMDD",
                    width: '140px'
                },{
                    type: 'text',
                    label: '代码',
                    prop: 'instrumentId',
                    width: '60px'
                },{
                    type: 'text',
                    label: '',
                    prop: 'side',
                    width: '35px'
                },{
                    type: 'text',
                    label: '',
                    prop: 'offset',
                    width: '40px'
                },{
                    type: 'number',
                    label: '成交价',
                    prop: 'price',
                    width: '80px'
                },{
                    type: 'number',
                    label: '成交量',
                    prop: 'volume',
                    width: '60px'
                },{
                    type: 'account-strategy',
                    label: this.moduleType == 'account' ? '策略': '账户',
                    prop: this.moduleType == 'account' ? 'clientId': 'accountId',
                }]
            }

            return [{
                type: 'text',
                label: '成交时间',
                prop: "updateTime",
                width: '70px'
            },{
                type: 'text',
                label: '代码',
                prop: 'instrumentId',
                width: '60px'
            },{
                type: 'text',
                label: '',
                prop: 'side',
                width: '35px'
            },{
                type: 'text',
                label: '',
                prop: 'offset',
                width: '40px'
            },{
                type: 'number',
                label: '成交价',
                prop: 'price',
                width: '80px'
            },{
                type: 'number',
                label: '成交量',
                prop: 'volume',
                width: '60px'
            },{
                type: 'number',
                label: "交易延迟(μs)",
                prop: "tradeLatency", 
                width: '90px'
            },{
                type: 'account-strategy',
                label: this.moduleType == 'account' ? '策略': '账户',
                prop: this.moduleType == 'account' ? 'clientId': 'accountId',
            }]
        }
    },

    watch: {
        kungfuData (trades) {
            const t = this;
            const tradesResolve = t.dealTradeList(trades, {
                searchKeyword: t.searchKeyword
            })
            t.tableData = tradesResolve
        }
    },

    methods:{
        dealTradeList (trades, { searchKeyword}) {
            const t = this;
            let tradesAfterFilter = trades
                .filter(item => {
                    if (searchKeyword.trim() === '') return true;
                    const { trade_id, client_id, account_id, source_id, instrument_id } = item
                    const strings = [ trade_id.toString(), client_id, account_id, source_id, instrument_id ].join('')
                    return strings.includes(searchKeyword) 
                })

            if (t.moduleType === 'strategy') {
                tradesAfterFilter = tradesAfterFilter
                    .filter(item => {
                        return Number(item.trade_time) >= t.addTime 
                    })
            }

            tradesAfterFilter = tradesAfterFilter
                .map(item => {
                    let tradeData = dealTrade(item);
                    let orderId = tradeData.orderId;
                    tradeData.update = !!t.tableData.length;
                    tradeData.tradeLatency = (t.orderStat[orderId] || {}).tradeLatency || '';
                    return tradeData
                })
                .sort((a, b) => (b.updateTimeNum - a.updateTimeNum))

            return Object.freeze(tradesAfterFilter || [])
        }
    }
}
</script>

<style lang="scss" scoped>

.trades-record{
    height: 100%;
}

.trading-day-header {
    font-size: 10px;
    padding-left: 10px;
}

</style>