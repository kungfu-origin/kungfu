<template>
    <main-content>
        <div class="trader-content">
                <el-col :span="14">
                    <el-row style="height: 33.33%">
                        <el-col :span="14">
                            <Strategy></Strategy>
                        </el-col>
                        <el-col :span="10">
                            <Pnl
                                ref="pnl"
                                :currentId="strategyId" 
                                moduleType="strategy"
                                :minMethod="getStrategyPnlMin"
                                :dayMethod="getStrategyPnlDay"
                                :nanomsgBackData="minNanomsgBack"
                                ></Pnl>
                        </el-col>
                    </el-row>
                    <el-row style="height: 66.66%">
                        <el-col>
                            <Log></Log>                         
                        </el-col>
                    </el-row>
                </el-col>
                <el-col  :span="10">
                    <el-row style="height: 33.333%">
                            <Pos
                            ref="pos"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :getDataMethod="getStrategyPos"
                            :nanomsgBackData="posNanomsgBack"                   
                            ></Pos>
                    </el-row>
                    <el-row  style="height: 33.333%">
                            <CurrentOrder
                            ref="current-order"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :getDataMethod="getStrategyOrder"
                            :nanomsgBackData="orderNanomsgBack"
                            ></CurrentOrder>                      
                    </el-row>
                    <el-row style="height: 33.333%">
                            <TradeRecord 
                            ref="trade-record"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :getDataMethod="getStrategyTrade"
                            :nanomsgBackData="tradeNanomsgBack"
                            ></TradeRecord>
                    </el-row>
                </el-col>

        </div>
    </main-content>
</template>
<script>
import Strategy from './components/Strategy';
import Log from './components/Log';
import CurrentOrder from '../Base/CurrentOrder';
import TradeRecord from '../Base/TradeRecord';
import Pos from '../Base/Pos';
import Pnl from '../Base/pnl/Pnl';
import {mapState, mapGetters} from 'vuex';
import * as STRATEGY_API from '__io/db/strategy'
import * as msgType from '__io/nano/msgType'


export default {
    data(){
        const t = this;
        return {
            orderNanomsgBack: null, //传入order组件中的nanomsg推送的数据
            posNanomsgBack: null, //传入pos组件中的nanomsg推送的数据
            tradeNanomsgBack: null, //传入trades组件中的nanomsg推送的数据
            minNanomsgBack: null, //传入min组件中的nanomsg推送的数据
        }
    },
 
   
    computed: {
        ...mapState({
            currentStrategy: state => state.STRATEGY.currentStrategy,
        }),

        strategyId(){
            return this.currentStrategy.strategy_id
        }    
    },

    components: {
        Strategy, CurrentOrder, TradeRecord, Pos, Log, Pnl 
    },

    methods:{
        
        getStrategyPos: STRATEGY_API.getStrategyPos,
        getStrategyOrder: STRATEGY_API.getStrategyOrder,
        getStrategyTrade: STRATEGY_API.getStrategyTrade,
        getStrategyPnlMin: STRATEGY_API.getStrategyPnlMin,
        getStrategyPnlDay: STRATEGY_API.getStrategyPnlDay

    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.trader-content{
    height: 100%;
}

.trader-content > .el-row{
    height: 100%;
}

.el-col{
    height: 100%;
}

.el-row > .tr-dashboard{
    padding-right: 0px;
}

.trader-content > .el-row:last-child .tr-dashboard{
    padding-bottom: 0px;
}

.el-col > .el-row:last-child .tr-dashboard{
    padding-bottom: 0px;
}
.el-col.has-padding .tr-dashboard{
    padding-bottom: 8px !important;
}
</style>
