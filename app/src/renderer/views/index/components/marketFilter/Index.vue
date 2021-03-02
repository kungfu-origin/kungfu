<template>
    <MainContent>

        <div class="market-filter-content">
            <el-row style="height: 55%">
                <el-col :span="3">
                    <TickerSet></TickerSet>
                </el-col>
                <el-col :span="17">
                    <MarketData 
                        :marketData="quoteData" 
                        @clickQuote="handleClickQuote"
                    ></MarketData>
                </el-col>
                <el-col :span="4"></el-col>
            </el-row>
            <el-row style="height: 45%">
                <el-col :span="6">
                    <MakeOrderDashboard
                        currentId=""
                        moduleType="ticker" 
                        :makeOrderByQuote="makeOrderByQuote"
                    ></MakeOrderDashboard>
                </el-col>               
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import MainContent from '@/components/Layout/MainContent';
import TickerSet from '@/components/MarketFilter/components/TickerSet';
import MarketData from '@/components/MarketFilter/components/MarketData';
import MakeOrderDashboard from '@/components/Base/makeOrder/MakeOrderDashboard';


import { buildMarketDataPipe } from '__io/kungfu/tradingData'; 

export default {

    components: {
        MainContent,
        TickerSet,
        MarketData,
        MakeOrderDashboard
    },

    data () {
        
        return {
            quoteData: {},
            makeOrderByQuote: {}
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

        handleClickQuote (quote) {
            this.makeOrderByQuote = Object.freeze(quote);
        }
    }
}
</script>

<style lang="scss" scoped>

.market-filter-content {
    height: 100%
}

.el-col {
    height: 100%;
}

</style>