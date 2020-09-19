<template>
    <main-content>
        <div class="trader-content" v-if="!(monitOrders || monitTrades)">
            <template v-if="monitStrategies">
                <el-col :span="14">
                    <el-row style="height: 60%">
                        <el-col>
                            <Strategy
                            v-model="monitStrategies"
                            ></Strategy>
                        </el-col>
                    </el-row>
                    <el-row style="height: 40%">
                        <el-col :span="14">
                            <Log></Log>                         
                        </el-col>
                        <el-col :span="10">
                            <Pnl
                            ref="pnl"
                            :currentId="strategyId" 
                            moduleType="strategy"
                            :minPnl="pnl"   
                            :dailyPnl="dailyPnl"
                            :addTime="addTime"                
                            ></Pnl>
                        </el-col>
                    </el-row>
                </el-col>
            </template>
            <template v-else>
                <el-col :span="14">
                    <el-row style="height: 33.33%">
                        <el-col :span="14">
                            <Strategy
                            v-model="monitStrategies"
                            ></Strategy>
                        </el-col>
                        <el-col :span="10">
                            <Pnl
                            ref="pnl"
                            :currentId="strategyId" 
                            moduleType="strategy"
                            :minPnl="pnl"   
                            :dailyPnl="dailyPnl"
                            :addTime="addTime"                
                            ></Pnl>
                        </el-col>
                    </el-row>

                    <el-row style="height: 66.66%">
                        <el-col>
                            <Log></Log>                         
                        </el-col>
                    </el-row>
                </el-col>
            </template>
                
            <el-col  :span="10">
                <el-row style="height: 33.333%">
                        <Pos
                        moduleType="strategy"
                        :currentId="strategyId"
                        :kungfuData="positions"   
                        :addTime="addTime" 
                        @showMakeOrderDashboard="handleShowOrCloseMakeOrderDashboard(true)"
                        @makeOrder="handleMakeOrderByPos"
                        ></Pos>
                </el-row>
                <el-row  style="height: 33.333%">
                        <CurrentOrder
                        moduleType="strategy"
                        v-model="monitOrders"
                        :currentId="strategyId"
                        :kungfuData="orders"                 
                        :addTime="addTime"   
                        :orderStat="orderStat"   
                        @showHistory="handleShowHistory"          
                        ></CurrentOrder>                      
                </el-row>
                <el-row style="height: 33.333%">
                        <TradeRecord 
                        moduleType="strategy"
                        v-model="monitTrades"
                        :currentId="strategyId"
                        :kungfuData="trades"           
                        :addTime="addTime"    
                        :orderStat="orderStat"
                        @showHistory="handleShowHistory"          
                        ></TradeRecord>
                </el-row>
            </el-col>
        </div>

        <div class="trader-content" v-else>
            <template>
                <el-col :span="10">
                    <el-row style="height: 33.33%">
                        <el-col>
                            <Strategy
                            v-model="monitStrategies"
                            ></Strategy>
                        </el-col>
                    </el-row>
                    <el-row style="height: 66.66%">
                        <el-col>
                            <Log></Log>                         
                        </el-col>
                    </el-row>
                </el-col>
            </template>
                
            <el-col :span="14">
                <el-row :style="{ height: '100%' }">
                        <CurrentOrder
                        v-if="monitOrders"
                        moduleType="strategy"
                        v-model="monitOrders"
                        :currentId="strategyId"
                        :kungfuData="orders"                 
                        :addTime="addTime"   
                        :orderStat="orderStat"   
                        @showHistory="handleShowHistory"          
                        ></CurrentOrder>    
                        <TradeRecord 
                        v-if="monitTrades"
                        moduleType="strategy"
                        v-model="monitTrades"
                        :currentId="strategyId"
                        :kungfuData="trades"           
                        :addTime="addTime"    
                        :orderStat="orderStat"
                        @showHistory="handleShowHistory"          
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
import accountStrategyMixins from '@/assets/js/mixins/accountStrategyMixins';

export default {
    mixins: [ accountStrategyMixins ],

    data(){
        this.tradingDataPipe = null;
        this.moduleType = 'strategy';

        return {
            orders: Object.freeze([]),
            trades: Object.freeze([]),
            positions: Object.freeze([]),
            pnl: Object.freeze([]),
            dailyPnl: Object.freeze([]),
            orderStat: Object.freeze({}),

            historyData: {},
            monitStrategies: false,
            monitOrders: false,
            monitTrades: false
        }
    },

    mounted(){
        const t = this;
        t.tradingDataPipe = buildTradingDataPipe('strategy').subscribe(data => {
            
            if (this.historyData['order'] && ((this.historyData['order'] || {}).date)) {
                this.orders = Object.freeze(this.historyData['order'].data)
            } else {
                const orders = data['orders'][t.strategyId];
                this.orders = Object.freeze(orders || []);
            }

            if (this.historyData['trade'] && ((this.historyData['trade'] || {}).date)) {
                this.trades = Object.freeze(this.historyData['trade'].data)
            } else {
                const trades = data['trades'][t.strategyId];
                this.trades = Object.freeze(trades || []);
            }
           
            const positions = data['positions'][t.strategyId];
            this.positions = Object.freeze(positions || []);
            const pnl = data['pnl'][t.strategyId];
            this.pnl = Object.freeze(pnl || []);
            const dailyPnl = data['dailyPnl'][t.strategyId];
            this.dailyPnl = Object.freeze(dailyPnl || []);
            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {});

            const assets = data['assets'];
            this.$store.dispatch('setStrategiesAsset', Object.freeze(JSON.parse(JSON.stringify(assets))));
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
        },
        
        addTime () {
            return this.currentStrategy.add_time
        }
    },

    components: {
        Strategy, CurrentOrder, TradeRecord, 
        Pos, Log, Pnl
    },

    methods:{

        handleShowHistory ({ date, data, type }) {
            this.$set(this.historyData, type, {
                date,
                data
            })
        }
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
