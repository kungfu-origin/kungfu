<template>
<tr-dashboard :title=" noTitle ? '' : `成交记录 ${currentTitle}`">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-tooltip
                class="item"
                effect="dark"
                content="实时最多显示100条数据，更多数据请通过查询历史查看"
                placement="right"
            >
                <i class="el-icon-question mouse-over"></i>
            </el-tooltip>        
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest && !dateForHistory">
            <i class="el-icon-date mouse-over" title="历史" @click="dateRangeDialogVisiblityForHistory = true"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest && dateForHistory">
            <span>{{ dateForHistory }}</span>
            <i class="el-icon-close mouse-over" @click="handleClearHistory"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-download mouse-over" title="导出" @click="dateRangeDialogVisiblityForExport = true"></i>
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

import DatePickerDialog from '@/components/Base/DatePickerDialog';

import { dealOrderStat } from '__io/kungfu/watcher';
import { tradesHeader } from '@/components/Base/tradingData/js/tableHeaderConfig';

import tradingDataMixin from '@/components/Base/tradingData/js/tradingDataMixin';

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
            let tradesAfterFilter = trades
                .filter(item => {
                    if (searchKeyword.trim() === '') return true;
                    const { tradeId, clientId, accountId, sourceId, instrumentId, orderId } = item
                    const strings = [ tradeId, clientId, accountId, sourceId, instrumentId, orderId ].join('')
                    return strings.includes(searchKeyword) 
                })

            if (this.moduleType === 'strategy') {
                tradesAfterFilter = tradesAfterFilter
                    .filter(item => {
                        return Number(item.updateTimeNum) >= this.addTime 
                    })
            }

            tradesAfterFilter = tradesAfterFilter
                .map(item => {
                    let tradeData = { ...item };
                    let orderId = tradeData.orderId;
                    const orderStatByOrderId = this.dateForHistory ? item : dealOrderStat(this.orderStat[orderId] || null);
                    tradeData.update = !!this.tableData.length;
                    tradeData.latencyTrade = orderStatByOrderId.latencyTrade || '';
                    //ctp trade返回的是交易所时间（xtp是自己维护），所用orderState内时间代替
                    tradeData.localUpdateTime = orderStatByOrderId.tradeTime
                    tradeData.localUpdateTimeNum = orderStatByOrderId.tradeTimeNum
                    tradeData.localUpdateTimeMMDD = orderStatByOrderId.tradeTimeMMDD
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