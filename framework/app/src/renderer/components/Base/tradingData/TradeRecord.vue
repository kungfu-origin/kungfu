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
        :data="tableDataResolved"
        :schema="schema"
        :renderCellClass="renderCellClass"
        @rightClickRow="handleShowDetail"
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

import DatePickerDialog from '__renderer/components/Base/DatePickerDialog';

import { dealOrderStat } from '@kungfu-trader/kungfu-uicc/io/kungfu/watcher';
import { tradesHeader } from '__renderer/components/Base/tradingData/js/tableHeaderConfig';

import tradingDataMixin from '__renderer/components/Base/tradingData/js/tradingDataMixin';

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
        },

        tableDataResolved () {
            if (this.searchKeyword) {
                return this.tableData.filter(item => {
                    if (this.searchKeyword.trim() === '') return true;
                    const { tradeId, clientId, accountId, sourceId, instrumentId, orderId } = item
                    const strings = [ tradeId, clientId, accountId, sourceId, instrumentId, orderId ].join('')
                    return strings.includes(this.searchKeyword) 
                })
            } else {
                return this.tableData
            }
        }
    },

    watch: {
        kungfuData (trades) {
            const tradesResolved = this.dealTradeList(trades)
            this.tableData = tradesResolved
        }
    },

    methods:{
        handleShowDetail (row) {
            let tradeData = JSON.parse(JSON.stringify(row));
            let orderMessage = "";

            delete tradeData.id;
            delete tradeData.source;
            delete tradeData.dest;
            delete tradeData.updateTimeNum;
            delete tradeData.updateTime;
            delete tradeData.update;
            delete tradeData.sourceId;
            delete tradeData.hedgeFlagOrigin;
            delete tradeData.localUpdateTimeNum;
            delete tradeData.localUpdateTimeMMDD;
            delete tradeData.sideOrigin;
            delete tradeData.instrumentTypeOrigin;

            Object.keys(tradeData || {}).forEach(key => {
                const value = tradeData[key];
                orderMessage += `${key}: ${value} </br>`
            })

            this.$alert(orderMessage, `成交详情 ${tradeData.orderId}`, {
                confirmButtonText: '确定',
                dangerouslyUseHTMLString: true,
                closeOnPressEscape: true,
                callback: () => {}
            });
        },

        dealTradeList (trades) {
            let tradesResolved = trades;

            if (this.moduleType === 'strategy') {
                tradesResolved = tradesResolved
                    .filter(item => {
                        return Number(item.updateTimeNum) >= this.addTime 
                    })
            }

            tradesResolved = tradesResolved
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

            return Object.freeze(tradesResolved || [])
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