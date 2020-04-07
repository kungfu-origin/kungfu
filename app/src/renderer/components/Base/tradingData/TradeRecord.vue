<template>
<tr-dashboard :title="`当日成交 ${currentTitle}`">
    <template v-slot:dashboard-header-left>
        <span class="trading-day-header">交易日：{{tradingDay}}</span>    
    </template>
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-refresh mouse-over" title="刷新" @click="handleRefresh"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-download mouse-over" title="导出" @click="dateRangeDialogVisiblity = true"></i>
        </tr-dashboard-header-item>
    </div>
    <tr-table
        v-if="rendererTable"
        :data="tableData"
        :schema="schema"
        :renderCellClass="renderCellClass"
    ></tr-table>
    <date-range-dialog 
    @confirm="handleConfirmDateRange"
    :visible.sync="dateRangeDialogVisiblity"    
    ></date-range-dialog>
</tr-dashboard>

</template>

<script>
import moment from 'moment'

import DateRangeDialog from '../DateRangeDialog';
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
        }
    },

    components: {
        DateRangeDialog
    },

    data () {
        return {
            kungfuBoardType: 'trade',
        }
    },

    computed:{
        schema(){
            return [{
                type: 'text',
                label: '成交时间',
                prop: 'updateTime',
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
                width: '30px'
            },{
                type: 'text',
                label: '',
                prop: 'offset',
                width: '30px'
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
                tradingDay: t.tradingDay,
                searchKeyword: t.searchKeyword
            })
            tradesResolve.length && (t.tableData = tradesResolve)
        }
    },

    methods:{
        dealTradeList (trades, { tradingDay, searchKeyword}) {
            const t = this;
            let tradesAfterFilter = trades
                .filter(item => (item.trading_day === tradingDay))
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