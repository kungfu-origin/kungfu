<template>
    <MainContent>
        <div class="trader-content">
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
                        <el-col>
                            <el-tabs type="border-card" v-model="currentStrategyDetailTab">
                                <el-tab-pane :lazy="true" :label="`策略日志 ${showCurrentIdInTabName(currentStrategyDetailTab, 'log')}`" name="log">
                                    <Log></Log>                         
                                </el-tab-pane>
                                <el-tab-pane :lazy="true" :label="`委托跟踪 ${showCurrentIdInTabName(currentStrategyDetailTab, 'orderMap')}`" name="orderMap">
                                    <OrderMap
                                        :orders="orders"
                                        :orderInputs="orderInputs"
                                        :trades="trades"
                                        :orderStat="orderStat"
                                    ></OrderMap>
                                </el-tab-pane>
                            </el-tabs>
                        </el-col>
                    </el-row>
                </el-col>
            </template>
            <template v-else>
                <el-col :span="14">
                    <el-row style="height: 33.33%">
                        <el-col>
                            <Strategy
                            v-model="monitStrategies"
                            ></Strategy>
                        </el-col>
                    </el-row>

                    <el-row style="height: 66.66%">
                        <el-col>
                            <el-tabs type="border-card" v-model="currentStrategyDetailTab">
                                <el-tab-pane :lazy="true" :label="`策略日志 ${showCurrentIdInTabName(currentStrategyDetailTab, 'log')}`" name="log">
                                    <Log></Log>                         
                                </el-tab-pane>
                                <el-tab-pane :lazy="true" :label="`委托跟踪 ${showCurrentIdInTabName(currentStrategyDetailTab, 'orderMap')}`" name="orderMap">
                                    <OrderMap
                                        :orders="orders"
                                        :orderInputs="orderInputs"
                                        :trades="trades"
                                        :orderStat="orderStat"
                                    ></OrderMap>
                                </el-tab-pane>
                            </el-tabs>
                        </el-col>
                    </el-row>
                </el-col>
            </template>
                
            <el-col  :span="10">
                <el-row style="height: 33.333%">
                    <el-tabs type="border-card" v-model="currentStrategyPosPnlTab">
                        <el-tab-pane :lazy="true" :label="`持仓 ${showCurrentIdInTabName(currentStrategyPosPnlTab, 'pos')}`" name="pos">
                            <Pos
                            :noTitle="true"
                            moduleType="strategy"
                            :currentId="strategyId"
                            :kungfuData="positions"   
                            :addTime="addTime" 
                            @showMakeOrderDashboard="handleShowOrCloseMakeOrderDashboard(true)"
                            @makeOrder="handleMakeOrderByPos"
                            ></Pos>
                        </el-tab-pane>
                        <el-tab-pane :lazy="true" :label="`盈利曲线 ${showCurrentIdInTabName(currentStrategyPosPnlTab, 'pnl')}`" name="pnl">
                            <Pnl
                            :noTitle="true"
                            ref="pnl"
                            :currentId="strategyId" 
                            moduleType="strategy"
                            :minPnl="pnl"   
                            :dailyPnl="dailyPnl"
                            :addTime="addTime"                
                            ></Pnl>
                        </el-tab-pane>
                    </el-tabs> 
                </el-row>
                <el-row  style="height: 33.333%">
                        <OrderRecord
                        moduleType="strategy"
                        :currentId="strategyId"
                        :kungfuData="orders"                 
                        :addTime="addTime"   
                        :orderStat="orderStat"   
                        @showHistory="handleShowHistory"          
                        ></OrderRecord>                      
                </el-row>
                <el-row style="height: 33.333%">
                        <TradeRecord 
                        moduleType="strategy"
                        :currentId="strategyId"
                        :kungfuData="trades"           
                        :addTime="addTime"    
                        :orderStat="orderStat"
                        @showHistory="handleShowHistory"          
                        ></TradeRecord>
                </el-row>
            </el-col>
        </div>
    </MainContent>
</template>
<script>

import { mapState } from 'vuex';

import Strategy from '@/components/Strategy/Strategy';
import Log from '@/components/Strategy/Log';
import OrderRecord from '@/components/Base/tradingData/OrderRecord';
import TradeRecord from '@/components/Base/tradingData/TradeRecord';
import OrderMap from "@/components/Base/tradingData/OrderMap";
import Pos from '@/components/Base/tradingData/Pos';
import Pnl from '@/components/Base/tradingData/pnl/Index';
import MainContent from '@/components/Layout/MainContent';

import { buildTradingDataStrategyPipeByDaemon } from '@/ipcMsg/daemon';

import accountStrategyMixins from '@/views/index/js/accountStrategyMixins';

export default {
    mixins: [ accountStrategyMixins ],

    data(){
        this.tradingDataPipe = null;
        this.moduleType = 'strategy';

        return {
            orders: Object.freeze([]),
            orderInputs: Object.freeze([]),
            trades: Object.freeze([]),
            positions: Object.freeze([]),
            pnl: Object.freeze([]),
            dailyPnl: Object.freeze([]),
            orderStat: Object.freeze({}),

            historyData: {},
            monitStrategies: false,
            currentStrategyDetailTab: "log",
            currentStrategyPosPnlTab: "pos",
        }
    },

    mounted(){
        this.tradingDataPipe = buildTradingDataStrategyPipeByDaemon().subscribe(data => {
            
            if (this.isHistoryData('order')) {
                this.orders = this.getHistoryData('order')
            } else {
                const orders = data['orders'][this.strategyId];
                this.orders = Object.freeze(orders || []);
            }

            if (this.isHistoryData('trade')) {
                this.trades = this.getHistoryData('trade')
            } else {
                const trades = data['trades'][this.strategyId];
                this.trades = Object.freeze(trades || []);
            }

            const orderInputs = data['orderInputs'][this.strategyId];
            this.orderInputs = Object.freeze(orderInputs);

            const positions = data['positions'][this.strategyId];
            this.positions = Object.freeze(positions || []);
  
            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {});

            const pnl = data['pnl'][this.strategyId];
            this.pnl = Object.freeze(pnl || []);
            const dailyPnl = data['dailyPnl'][this.strategyId];
            this.dailyPnl = Object.freeze(dailyPnl || []);

            const assets = data['assets'];
            this.$store.dispatch('setStrategiesAsset', Object.freeze(assets));
        })
    },

    destroyed(){
        this.tradingDataPipe && this.tradingDataPipe.unsubscribe();
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
        Strategy, OrderRecord, OrderMap, TradeRecord, 
        Pos, Log, Pnl,
        MainContent
    },

    methods:{
        showCurrentIdInTabName (currentTabName, target) {
            return currentTabName === target ? this.strategyId : ''
        },
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

.el-row > .el-tabs {

    .tr-dashboard{
        padding-right: 0px;
    }
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
