<template>
    <MainContent>
        <div class="account-content">
            <el-row style="height: 55%">
                <el-col :span="14">
                    <el-tabs :value="currentAccountTabName" type="border-card" @tab-click="handleAccountTabClick">
                        <el-tab-pane :lazy="true" label="账户列表" name="tdList">
                            <TdAccount></TdAccount>
                        </el-tab-pane>
                        <el-tab-pane :lazy="true" label="行情源" name="mdList">
                            <MdAccount></MdAccount>
                        </el-tab-pane>
                        <el-tab-pane :lazy="true" label="交易标的" name="tickerList">
                            <Pos 
                            :noTitle="true"
                            moduleType="acocunt"
                            :kungfuData="positionsByTicker"
                            :currentTicker="currentTickerResolved"
                            @activeTicker="setCurrentTicker"
                            />
                        </el-tab-pane>
                        <el-tab-pane :lazy="false" v-if="proMode" label="算法任务" name="tradingTask" >
                            <Task :noTitle="true"></Task>
                        </el-tab-pane>
                    </el-tabs>
                </el-col>
                <el-col :span="10">
                    <el-row style="height: 45%">
                        <Pos 
                        :moduleType="moduleType"
                        :currentId="currentId" 
                        :accountType="accountType"
                        :kungfuData="positions"
                        @showMakeOrderDashboard="handleShowOrCloseMakeOrderDashboard(true)"
                        @makeOrder="handleMakeOrderByPos"
                        />
                    </el-row>
                    <el-row style="height: 55%" class="has-padding-bottom">
                        <el-tabs type="border-card" v-model="currentTradesPnlTabNum">
                            <el-tab-pane :lazy="true" :label="`成交记录 ${showCurrentIdInTabName(currentTradesPnlTabNum, 'trades')}`" name="trades">
                                <TradeRecord
                                :noTitle="true"
                                :moduleType="moduleType" 
                                :currentId="currentId"
                                :kungfuData="trades"
                                :orderStat="orderStat"
                                @showHistory="handleShowHistory"
                                />
                            </el-tab-pane>
                            <el-tab-pane :disabled="moduleType === 'ticker'" :lazy="true" :label="`盈利曲线 ${showCurrentIdInTabName(currentTradesPnlTabNum, 'pnl')}`" name="pnl">
                                <Pnl 
                                :noTitle="true"
                                :currentId="currentId" 
                                :moduleType="moduleType"
                                :minPnl="pnl"
                                :dailyPnl="dailyPnl"
                                />
                            </el-tab-pane>
                        </el-tabs>
                    </el-row>
                </el-col>
            </el-row>
            <el-row style="height: 45%">
                <el-col :span="18">
                    <el-tabs v-model="currentOrdesTabName" type="border-card">
                        <el-tab-pane :lazy="true" :label="`全部委托 ${showCurrentIdInTabName(currentOrdesTabName, 'orders')}`" name="orders">
                            <OrderRecord
                            :noTitle="true"
                            :moduleType="moduleType" 
                            :todayFinishPreSetting="true"
                            :currentId="currentId"
                            :kungfuData="orders"
                            :gatewayName="currentAccount.account_id"
                            :orderStat="orderStat"
                            @showHistory="handleShowHistory"
                            />   
                        </el-tab-pane>
                        <el-tab-pane :lazy="true" :label="`未完成委托 ${showCurrentIdInTabName(currentOrdesTabName, 'unfinishedOrders')}`" name="unfinishedOrders">
                            <OrderRecord
                            :noTitle="true"
                            :moduleType="moduleType" 
                            :todayFinishPreSetting="false"
                            :currentId="currentId"
                            :kungfuData="orders"
                            :gatewayName="currentAccount.account_id"
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
                        :currentId="currentId"
                        :moduleType="moduleType" 
                        :makeOrderByPosData="makeOrderByPosData"
                    ></MakeOrderDashboard>
                </el-col>
            </el-row>
        </div>
    </MainContent>
</template>

<script>
import { mapGetters, mapState } from 'vuex'

import TdAccount from '@/components/Account/components/TdAccount';
import MdAccount from '@/components/Account//components/MdAccount';
import Task from '@/components/Task/Index';
import OrderRecord from '@/components/Base/tradingData/OrderRecord';
import TradeRecord from '@/components/Base/tradingData/TradeRecord';
import Pos from '@/components/Base/tradingData/Pos';
import Pnl from '@/components/Base/tradingData/pnl/Index';
import MakeOrderDashboard from '@/components/Base/makeOrder/MakeOrderDashboard';
import MainContent from '@/components/Layout/MainContent';
import TaskRecord from '@/components/Task/TaskRecord';

import { transformPositionByTickerByMerge } from '__io/kungfu/watcher';
import { orderTradesFilterByInstrumentIdDirection } from '__gUtils/busiUtils';
import { buildTradingDataAccountPipeByDaemon } from '@/ipcMsg/daemon';
import { buildOrderStatDataPipe } from '__io/kungfu/tradingData';

import accountStrategyMixins from '@/views/index/js/accountStrategyMixins';

export default {
    name: 'account',

    mixins: [ accountStrategyMixins ],

    data() {
        this.tradingDataPipe = null;

        return {
            orders: Object.freeze([]),
            trades: Object.freeze([]),
            pnl: Object.freeze([]),
            dailyPnl: Object.freeze([]),
            positions: Object.freeze([]),
            positionsByTicker: Object.freeze([]),
            orderStat: Object.freeze({}),

            currentOrdesTabName: "orders",
            currentTradesPnlTabNum: "trades",
        }
    },

    components: {
        TdAccount, MdAccount, Pos,
        Pnl,
        Task,
        OrderRecord, TradeRecord,
        MakeOrderDashboard,
        MainContent,
        TaskRecord
    },

    computed:{
        ...mapState({
            currentAccount: state => state.ACCOUNT.currentAccount, //选中的账户
            currentTicker: state => state.ACCOUNT.currentTicker,
            currentAccountTabName: state => state.ACCOUNT.currentAccountTabName,
            currentTaskId: state => (state.BASE.currentTask || {}).name || '',
            tdAccountSource: state => state.BASE.tdAccountSource || {},
            taskExtConfigList: state => state.BASE.taskExtConfigList || [],
        }),

        ...mapGetters([
            "proMode"
        ]),

        currentTickerResolved () {
            if (this.currentAccountTabName === 'tickerList') {
                return this.currentTicker
            } else {
                return null
            }
        },

        //账户的类型，根据是哪个柜台的，可以判断是是期货还是股票还是证券
        accountType() {
            const source_name = this.currentAccount.source_name
            if(!source_name) return
            return (this.tdAccountSource[source_name] || {}).typeName || ''
        },

        moduleType () {
            if (this.currentAccountTabName === 'tickerList') {
                return 'ticker'
            } else {
                return 'account'
            }
        },

        currentId () {
            if (this.moduleType === 'ticker') {
                return `${this.currentTicker.instrumentId || ''} ${this.currentTicker.direction || ''}`
            } else {
                return this.currentAccount.account_id || ''
            }
        },

        currentTickerId () {
            return `${this.currentTicker.instrumentId}_${this.currentTicker.directionOrigin}`
        },

        currentTaskIdInTab () {
            if (this.currentOrdesTabName === 'taskDetail') {
                return this.currentTaskId
            }

            return ''
        }
    },

    watch: {

        moduleType (val) {
            if (val === 'ticker') {
                this.currentTradesPnlTabNum = 'trades'
            }
        },
    },

    mounted ( ) {
        this.tradingDataPipe = buildTradingDataAccountPipeByDaemon().subscribe(data => {
            if (this.moduleType === 'ticker') {
                this.dealTradingDataByTiker(data)
            } else {
                this.dealTradingData(data);
            }

            const positionsByTicker = data['positionsByTicker'] || {};
            this.positionsByTicker = Object.freeze(transformPositionByTickerByMerge(positionsByTicker, 'account') || []);
            this.initSetCurrentTicker(this.positionsByTicker);
        })

        
        this.orderStatPipe = buildOrderStatDataPipe().subscribe(data => {
            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {});
        })
    },

    destroyed ( ) {
        this.tradingDataPipe && this.tradingDataPipe.unsubscribe();
        this.orderStatPipe && this.orderStatPipe.unsubscribe();
    },

    methods: {

        handleAccountTabClick (tab) {
            this.$store.dispatch('setCurrentAccountTabName', tab.name)
        },

        showCurrentIdInTabName (currentTabName, target) {
            return currentTabName === target ? this.currentId : ''
        },

        setCurrentTicker (item) {
            this.$store.dispatch('setCurrentTicker', item)
        },

        initSetCurrentTicker (tickerList) {
            if (!this.currentTicker || !this.currentTicker.instrumentId) {
                if (tickerList.length) {
                    const tickerListSort = tickerList.slice(0).sort((a, b) => {
                        const aid = a.instrumentId || ''
                        const bid = b.instrumentId || ''
                        const ad = a.direction || '';
                        const bd = b.direction || '';
                        const result = aid.localeCompare(bid);
                        return result === 0 ? ad.toString().localeCompare(bd.toString()) : result;
                    })

                    if (tickerListSort.length) {
                        this.$store.dispatch('setCurrentTicker', tickerListSort[0])
                    }
                }
            }
        },

        dealTradingData (data) {

            if (this.isHistoryData('order')) {
                this.orders = this.getHistoryData('order')
            } else {
                const orders = data['orders'][this.currentId];
                this.orders = Object.freeze(orders || []);
            }

            if (this.isHistoryData('trade')) {
                this.trades = this.getHistoryData('trade')
            } else {
                const trades = data['trades'][this.currentId];
                this.trades = Object.freeze(trades || []);
            }
      
            const positions = data['positions'][this.currentId];
            this.positions = Object.freeze(positions || []);

            const pnl = data['pnl'][this.currentId];
            this.pnl = Object.freeze(pnl || []);
            const dailyPnl = data['dailyPnl'][this.currentId];
            this.dailyPnl = Object.freeze(dailyPnl || []);

            const assets = data['assets'];
            this.$store.dispatch('setAccountsAsset', Object.freeze(assets));

        },


        dealTradingDataByTiker (data) {
            const { instrumentId, directionOrigin } = this.currentTicker;

            if (!instrumentId) {
                this.orders = Object.freeze([]);
                this.trades = Object.freeze([]);
                this.positions = Object.freeze([]);
                return 
            }
            const orders = this.isHistoryData('order') ? this.getHistoryData('order') : data['ordersByTicker']
            const ordersResolved = orders
                .filter(item => {
                    return orderTradesFilterByInstrumentIdDirection(item, instrumentId, directionOrigin)
                })
                .map(item => Object.freeze(item))
            this.orders = Object.freeze(ordersResolved || []);

            const trades = this.isHistoryData('trade') ? this.getHistoryData('trade') : data['tradesByTicker']
            const tradesResolved = trades
                .filter(item => {
                    return orderTradesFilterByInstrumentIdDirection(item, instrumentId, directionOrigin)
                })
                .map(item => Object.freeze(item))
            this.trades = Object.freeze(tradesResolved || []);

            const positionsByTicker = data['positionsByTicker'][this.currentTickerId] || [];
            const positionsByTickerForAccount = positionsByTicker.filter(item => !!item.accountId && !item.clientId);
            this.positions = Object.freeze(positionsByTickerForAccount)
        },

 
    },


 
}
</script>

<style lang="scss" scoped>

@import '@/assets/scss/skin.scss';

.account-content{
    height: 100%;
}

.account-content>.el-row{
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

</style>

