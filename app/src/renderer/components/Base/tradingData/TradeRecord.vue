<template>
<tr-dashboard :title=" noTitle ? '' : `成交记录 ${currentTitle}`">
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

import DatePickerDialog from '@/components/Base//DatePickerDialog';
import tradingDataMixin from '@/components/Base/tradingData/js/tradingDataMixin';

import { debounce } from "__gUtils/busiUtils";
import { dealTrade } from "__io/kungfu/watcher";
import { tradesHeader } from '@/components/Base/tradingData/js/tableHeaderConfig';

export default {
    name: 'trades-record',

    mixins: [ tradingDataMixin ],

    props: {
        orderStat: {
            type: Object,
            default: () => ({})
        },
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
            return tradesHeader(this.dateForHistory, this.moduleType)
        }
    },

    watch: {
        kungfuData (trades) {
            const tradesResolve = this.dealTradeList(trades, {
                searchKeyword: this.searchKeyword
            })
            this.tableData = tradesResolve
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
                    tradeData.latencyTrade = (t.orderStat[orderId] || {}).latencyTrade || '';
                    return Object.freeze(tradeData)
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