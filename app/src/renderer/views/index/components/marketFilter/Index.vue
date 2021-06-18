<template>
    <MainContent>

        <div class="market-filter-content">
            <el-row style="height: 55%">
                <el-col :span="6">
                    <TickerSet></TickerSet>
                </el-col>
                <el-col :span="14">
                    <el-tabs v-model="currentMarketDataTabName" type="border-card">
                        <el-tab-pane :lazy="true" :label="`行情明细 ${currentTickerSetNameInTab}`" name="marketData">
                            <MarketData 
                                :marketData="quoteData" 
                                @clickQuote="handleClickQuote"
                            ></MarketData>
                        </el-tab-pane>
                        <el-tab-pane :lazy="true" label="行情源" name="md">
                            <MdAccount></MdAccount>
                        </el-tab-pane>
                        <el-tab-pane :lazy="false" label="算法任务" name="tradingTasks"  v-if="taskExtConfigList.length">
                            <Task 
                            :noTitle="true" 
                            ></Task>
                        </el-tab-pane>
                    </el-tabs>
                </el-col>
                <el-col :span="4">
                    <OrderBook
                        :marketData="quoteData"
                        :tickerId="getTickerId(selectedQuote)"
                        @makeOrder="handleMakeOrderByOrderBook"
                    ></OrderBook>
                </el-col>
            </el-row>
            <el-row style="height: 45%">
                <el-col :span="18">
                    <el-tabs v-model="currentTradeDataTabName" type="border-card">
                        <el-tab-pane :lazy="true" label="全部委托" name="systemOrders">
                            <OrderRecord
                            :noTitle="true"
                            moduleType="all" 
                            :todayFinishPreSetting="true"
                            :kungfuData="orders"
                            :orderStat="orderStat"
                            @showHistory="handleShowHistory"
                            />  
                        </el-tab-pane>
                        <el-tab-pane :lazy="true"  v-if="proMode" :label="`算法任务记录 ${currentTaskIdInTab}`" name="taskDetail">
                            <TaskRecord></TaskRecord>
                        </el-tab-pane>
                    </el-tabs>
                </el-col>
                <el-col :span="6">
                    <MakeOrderDashboard
                        :currentId="(selectedQuote || {}).instrumentId || ''"
                        moduleType="ticker" 
                        :makeOrderByQuote="selectedQuote"
                    ></MakeOrderDashboard>
                </el-col>        
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import { mapGetters, mapState } from 'vuex';

import MainContent from '@/components/Layout/MainContent';
import TickerSet from '@/components/MarketFilter/components/TickerSet';
import MdAccount from '@/components/Account//components/MdAccount';
import OrderBook from '@/components/MarketFilter/components/OrderBook';
import MarketData from '@/components/MarketFilter/components/MarketData';
import MakeOrderDashboard from '@/components/Base/makeOrder/MakeOrderDashboard';
import Task from '@/components/Task/Index';
import TaskRecord from '@/components/Task/TaskRecord';
import OrderRecord from '@/components/Base/tradingData/OrderRecord';

import { watcher, dealOrder } from '__io/kungfu/watcher';
import { buildMarketDataPipeByDaemon } from '@/ipcMsg/daemon'; 
import { ensureLedgerData } from '__gUtils/busiUtils';
import { buildOrderStatDataPipe, buildAllOrdersTradesDataPipe } from '__io/kungfu/tradingData';

import accountStrategyMixins from '@/views/index/js/accountStrategyMixins';

export default {

    mixins: [ accountStrategyMixins ],

    components: {
        MainContent,
        TickerSet,
        MarketData,
        OrderBook,
        MakeOrderDashboard,
        Task,
        OrderRecord,
        MdAccount,
        TaskRecord
    },

    data () {
        return {
            quoteData: {},
            selectedQuote: {},
            currentMarketDataTabName: 'marketData',
            currentTradeDataTabName: 'systemOrders',
            orders: Object.freeze([]),
            orderStat: Object.freeze({}),
        }
    },

    computed: {
        ...mapState({
            currentTickerSetName: state => (state.MARKET.currentTickerSet || {}).name || '',
            currentTaskId: state => (state.BASE.currentTask || {}).name || '',
            taskExtConfigList: state => state.BASE.taskExtConfigList || []
        }),

        ...mapGetters([
            "proMode"
        ]),

        currentTickerSetNameInTab () {
            if (this.currentMarketDataTabName === 'marketData') {
                return this.currentTickerSetName
            }
            
            return ''
        },

        currentTaskIdInTab () {
            if (this.currentTradeDataTabName === 'taskDetail') {
                return this.currentTaskId
            }

            return ''
        },


    },

    mounted () {
        this.marketDataPipe = buildMarketDataPipeByDaemon().subscribe(data => {
            this.quoteData = Object.freeze(data);
        })

        this.allOrdersPipe = buildAllOrdersTradesDataPipe().subscribe(() => {
            if (this.isHistoryData('order')) {
                this.orders = this.getHistoryData('order')
            } else {
                const orders = ensureLedgerData(watcher.ledger.Order, 'update_time').slice(0, 100)
                const ordersResolved = orders.map(item => Object.freeze(dealOrder(item)));
                this.orders = Object.freeze(ordersResolved || []);
            }
        })

        this.orderStatPipe = buildOrderStatDataPipe().subscribe(data => {
            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {});
        })

        
    },
    
    destroyed(){
        this.marketDataPipe && this.marketDataPipe.unsubscribe();
        this.allOrdersPipe && this.allOrdersPipe.unsubscribe();
        this.orderStatPipe && this.orderStatPipe.unsubscribe();
    },

    methods: {

        handleMakeOrderByOrderBook (quote) {
            this.selectedQuote = Object.freeze(quote);
        },

        handleClickQuote (quote) {
            this.selectedQuote = Object.freeze(quote);
        },

        getTickerId (quote) {
            return `${(quote || {}).exchangeId || ''}_${(quote || {}).instrumentId || ''}_${(quote || {}).sourceId || ''}`
        }
    }
}
</script>

<style lang="scss" scoped>

.market-filter-content {
    height: 100%;

    .el-row{
        height: 50%;

        >.tr-dashboard{
            padding-right: 0px;
        }

        &:last-child .tr-dashboard{
            padding-bottom: 0px;
        }

        .el-col:last-child {

            .tr-dashboard {
                padding-right: 0;
            }
        }
    }

    .el-row {

        &.has-padding-bottom {

            .tr-dashboard {
                padding-bottom: 8px !important;
            }
        }
    }

    .el-col {
        height: 100%;
    }
}


</style>