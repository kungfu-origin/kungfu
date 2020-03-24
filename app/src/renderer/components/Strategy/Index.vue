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
                                :minPnl="pnl"   
                                :dayMethod="getStrategyPnlDay"
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
                            :currentId="strategyId"
                            moduleType="strategy"
                            :kungfuData="positions"   
                            :addTime="addTime"                
                            ></Pos>
                    </el-row>
                    <el-row  style="height: 33.333%">
                            <CurrentOrder
                            :currentId="strategyId"
                            moduleType="strategy"
                            :kungfuData="orders"     
                            :tradingDay="tradingDay"              
                            :addTime="addTime"   
                            :orderStat="orderStat"             
                            ></CurrentOrder>                      
                    </el-row>
                    <el-row style="height: 33.333%">
                            <TradeRecord 
                            :currentId="strategyId"
                            moduleType="strategy"
                            :kungfuData="trades"    
                            :tradingDay="tradingDay"          
                            :addTime="addTime"                
                            ></TradeRecord>
                    </el-row>
                </el-col>

        </div>
    </main-content>
</template>
<script>

import { mapState, mapGetters } from 'vuex';

import Strategy from './components/Strategy';
import Log from './components/Log';
import CurrentOrder from '../Base/tradingData/CurrentOrder';
import TradeRecord from '../Base/tradingData/TradeRecord';
import Pos from '../Base/tradingData/Pos';
import Pnl from '../Base/tradingData/pnl/Index';

import { buildTradingDataPipe } from '__io/kungfu/tradingData';

export default {
    data(){
        this.tradingDataPipe = null;
        return {
            orders: Object.freeze([]),
            trades: Object.freeze([]),
            positions: Object.freeze([]),
            pnl: Object.freeze([]),
            dailyPnl: Object.freeze([]),
            orderStat: Object.freeze({})
        }
    },

    mounted(){
        const t = this;
        t.tradingDataPipe = buildTradingDataPipe('strategy').subscribe(data => {
            const orders = data['orders'][t.strategyId];
            this.orders = Object.freeze(orders || []);
            const trades = data['trades'][t.strategyId];
            this.trades = Object.freeze(trades || []);
            const positions = data['positions'][t.strategyId];
            this.positions = Object.freeze(positions || []);
            const pnl = data['pnl'][t.strategyId];
            this.pnl = Object.freeze(pnl || []);
            const dailyPnl = data['dailyPnl'][t.currentId];
            this.dailyPnl = Object.freeze(dailyPnl || []);
            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {})
        })
    },

    destroyed(){
        const t = this;
        t.tradingDataPipe && t.tradingDataPipe.unsubscribe();
    },
   
    computed: {
        ...mapState({
            tradingDay: state => state.BASE.tradingDay, //日期信息，包含交易日
            currentStrategy: state => state.STRATEGY.currentStrategy,
        }),

        strategyId(){
            return this.currentStrategy.strategy_id
        },
        
        addTime () {
            return this.currentStrategy.add_time
        }
    },

    components: {
        Strategy, CurrentOrder, TradeRecord, Pos, Log, Pnl 
    },

    methods:{
        getStrategyPnlDay: () => { return Promise.resolve([]) }
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
