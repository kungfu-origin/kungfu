<template>
    <MainContent>

        <div class="market-filter-content">
            <el-row style="height: 55%">
                <el-col :span="4">
                    <TickerSet></TickerSet>
                </el-col>
                <el-col :span="16">
                    <MarketData 
                        :marketData="quoteData" 
                        @clickQuote="handleClickQuote"
                    ></MarketData>
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
                <el-col :span="6">
                    <MakeOrderDashboard
                        :currentId="(selectedQuote || {}).instrumentId || ''"
                        moduleType="ticker" 
                        :makeOrderByQuote="selectedQuote"
                    ></MakeOrderDashboard>
                </el-col>        
                <el-col :span="6">
                    <Task 
                    :noTitle="false" 
                    :selectable="true"
                    ></Task>
                </el-col>
                <el-col :span="12">
                    <el-tabs v-model="currentMarketFilterMultiFuncTabName" type="border-card">
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
                        <el-tab-pane :lazy="true" :label="`任务实盘 ${currentTaskId}`" name="taskReal">
                            111
                        </el-tab-pane>
                        <el-tab-pane :lazy="true" :label="`任务模拟 ${currentTaskId}`" name="taskTest">
                            asd
                        </el-tab-pane>
                    </el-tabs>
                </el-col>
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import { mapState } from 'vuex';

import MainContent from '@/components/Layout/MainContent';
import TickerSet from '@/components/MarketFilter/components/TickerSet';
import OrderBook from '@/components/MarketFilter/components/OrderBook';
import MarketData from '@/components/MarketFilter/components/MarketData';
import MakeOrderDashboard from '@/components/Base/makeOrder/MakeOrderDashboard';
import Task from '@/components/Task/Index';
import OrderRecord from '@/components/Base/tradingData/OrderRecord';

import { buildMarketDataPipe, buildAllOrdersPipe } from '__io/kungfu/tradingData'; 

export default {

    components: {
        MainContent,
        TickerSet,
        MarketData,
        OrderBook,
        MakeOrderDashboard,
        Task,
        OrderRecord
    },

    data () {
        
        return {
            quoteData: {},
            selectedQuote: {},
            currentMarketFilterMultiFuncTabName: 'systemOrders',
            orders: Object.freeze([]),
            orderStat: Object.freeze({}),

            historyData: {},
        }
    },

    computed: {
        ...mapState({
            currentTaskId: state => (state.BASE.currentTask || {}).name || ''
        })
    },

    mounted () {
        this.marketDataPipe = buildMarketDataPipe().subscribe(data => {
            this.quoteData = Object.freeze(data);
        })

        this.allOrdersPipe = buildAllOrdersPipe().subscribe(data => {
            const { orders, orderStat } = data;
            if (this.historyData['order'] && ((this.historyData['order'] || {}).date)) {
                this.orders = Object.freeze(this.historyData['order'].data)
            } else {
                const orders = data['orders'];
                this.orders = Object.freeze(orders || []);
            }
            this.orderStat = Object.freeze(orderStat || {});
        })
    },
    
    destroyed(){
        this.marketDataPipe && this.marketDataPipe.unsubscribe();
        this.allOrdersPipe && this.allOrdersPipe.unsubscribe();
    },

    methods: {
        handleShowHistory ({ date, data, type }) {
            this.$set(this.historyData, type, {
                date,
                data
            })
        },

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