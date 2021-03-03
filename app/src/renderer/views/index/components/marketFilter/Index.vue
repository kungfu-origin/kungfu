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
                <el-col :span="18">
                    <el-tabs v-model="currentMarketFilterMultiFuncTabName" type="border-card">
                        <el-tab-pane :lazy="true" label="策略筛选" name="strategy-filter">
                            asd
                        </el-tab-pane>
                    </el-tabs>
                </el-col>
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import MainContent from '@/components/Layout/MainContent';
import TickerSet from '@/components/MarketFilter/components/TickerSet';
import OrderBook from '@/components/MarketFilter/components/OrderBook';
import MarketData from '@/components/MarketFilter/components/MarketData';
import MakeOrderDashboard from '@/components/Base/makeOrder/MakeOrderDashboard';


import { buildMarketDataPipe } from '__io/kungfu/tradingData'; 

export default {

    components: {
        MainContent,
        TickerSet,
        MarketData,
        OrderBook,
        MakeOrderDashboard
    },

    data () {
        
        return {
            quoteData: {},
            selectedQuote: {},
            currentMarketFilterMultiFuncTabName: 'strategy-filter'
        }
    },

    mounted () {
        this.marketDataPipe = buildMarketDataPipe().subscribe(data => {
            this.quoteData = Object.freeze(data);
        })
    },
    
    destroyed(){
        this.marketDataPipe && this.marketDataPipe.unsubscribe();
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