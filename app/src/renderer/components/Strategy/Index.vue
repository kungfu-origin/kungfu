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
                                :nanomsgBackData="minPnlFromNmsg"
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
                            :nanomsgBackData="posFromNmsg"                   
                            ></Pos>
                    </el-row>
                    <el-row  style="height: 33.333%">
                            <CurrentOrder
                            ref="current-order"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :getDataMethod="getStrategyOrder"
                            :nanomsgBackData="ordersFromNmsg"
                            ></CurrentOrder>                      
                    </el-row>
                    <el-row style="height: 33.333%">
                            <TradeRecord 
                            ref="trade-record"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :getDataMethod="getStrategyTrade"
                            :nanomsgBackData="tradesFromNmsg"
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
import { mapState, mapGetters } from 'vuex';
import * as STRATEGY_API from '__io/db/strategy';
import * as MSG_TYPE from '__io/nano/msgType';
import { buildCashPipe, buildTradingDataPipe } from '__io/nano/nanoSub';


export default {
    data(){
        this.tradingDataPipe = null;
        return {
            ordersFromNmsg: null,
            tradesFromNmsg: null,
            posFromNmsg: null,
            minPnlFromNmsg: null
        }
    },
 
    mounted(){
        const t = this;
        t.tradingDataPipe = buildTradingDataPipe().subscribe(d => {
            const msgType = d.msg_type;
            const tradingData = d.data;
            const ledgerCategory = tradingData.ledger_category;
            const strategyId = tradingData.client_id || '';
            switch (msgType) {
                case MSG_TYPE.order:
                    if(strategyId !== t.strategyId) return;
                    t.ordersFromNmsg = Object.freeze(tradingData);
                    break
                case MSG_TYPE.trade:
                    if(strategyId !== t.strategyId) return;
                    t.tradesFromNmsg = Object.freeze(tradingData);
                    break
                case MSG_TYPE.position:
                    if(strategyId !== t.strategyId) return;
                    if(ledgerCategory !== 1) return;
                    t.posFromNmsg = Object.freeze(tradingData);
                    break
                case MSG_TYPE.portfolio:
                    if(strategyId !== currentId) return;
                    if(ledgerCategory !== 1) return;
                    t.minPnlFromNmsg = Object.freeze(tradingData);
            }
        })
    },

    destroyed(){
        const t = this;
        t.tradingDataPipe && t.tradingDataPipe.unsubscribe();
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
