<template>
<tr-dashboard :title="`当日成交 ${currentTitle}`">
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

import { debounce, dealTrade } from "__gUtils/busiUtils"
import { writeCSV } from '__gUtils/fileUtils';

export default {
    name: 'trades-record',

    mixins: [ tradingDataMixin ],

    components: {
        DateRangeDialog
    },

    computed:{
        schema(){
            return [{
                type: 'text',
                label: '成交时间',
                prop: 'updateTime',
                width: '160px'
            },{
                type: 'text',
                label: '代码',
                prop: 'instrumentId',
                width: '80px'
            },{
                type: 'text',
                label: '买卖',
                prop: 'side',
                width: '50px'
            },{
                type: 'text',
                label: '开平',
                prop: 'offset',
                width: '50px'
            },{
                type: 'number',
                label: '成交价',
                prop: 'price',
            },{
                type: 'number',
                label: '成交量',
                prop: 'volume',
            },{
                type: 'text',
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
                tradesAfterFilter = tradesAfterFilter.filter(item => Number(item.update_time) >= t.addTime )
            }

            tradesAfterFilter = tradesAfterFilter
                .map(item => {
                    let tradeData = dealTrade(item);
                    tradeData.update = !!t.tableData.length;
                    return tradeData
                })
                .sort((a, b) => (b.updateTimeNum - a.updateTimeNum))

            return Object.freeze(tradesAfterFilter || [])
        }
    }
}
</script>

<style lang="scss">
.trades-record{
    height: 100%;
}
</style>