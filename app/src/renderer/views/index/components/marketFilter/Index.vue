<template>
    <MainContent>

        <div class="market-filter-content">
            <el-row style="height: 55%">
                <el-col :span="5">
                    <TickerSet></TickerSet>
                </el-col>
                <el-col :span="15">
                    <MarketData :marketData="quoteData"></MarketData>
                </el-col>
                <el-col :span="4"></el-col>
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import MainContent from '@/components/Layout/MainContent';
import TickerSet from '@/components/MarketFilter/components/TickerSet';
import MarketData from '@/components/MarketFilter/components/MarketData';

import { buildMarketDataPipe } from '__io/kungfu/tradingData'; 

export default {

    components: {
        MainContent,
        TickerSet,
        MarketData
    },

    data () {
        
        return {
            quoteData: {}
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