<template>
    <MainContent>
        <div class="account-content">
            <el-row style="height: 50%">
                <el-col :span="16">
                    <el-tabs :value="currentAccountTabName" type="border-card" @tab-click="handleAccountTabClick">
                        <el-tab-pane label="账户列表" name="tdList">
                            <TdAccount></TdAccount>
                        </el-tab-pane>
                        <el-tab-pane label="行情源" name="mdList">
                            <MdAccount></MdAccount>
                        </el-tab-pane>
                        <el-tab-pane label="交易标的" name="tickerList">
                            <Pos 
                            :noTitle="true"
                            moduleType="account"
                            :currentId="currentId" 
                            :accountType="accountType"
                            :kungfuData="positionsByTicker"
                            :currentTicker="currentTickerResolved"
                            @activeTicker="setCurrentTicker"
                            />
                        </el-tab-pane>
                    </el-tabs>
                </el-col>
                <el-col :span="8">
                    <Pos 
                    moduleType="account"
                    :currentId="currentId" 
                    :accountType="accountType"
                    :kungfuData="positions"
                    @showMakeOrderDashboard="handleShowOrCloseMakeOrderDashboard(true)"
                    @makeOrder="handleMakeOrderByPos"
                    />
                </el-col>
            </el-row>
            <el-row style="height: 50%" class="flex-row">
                <el-col :span="7" :style="{ 'max-width': '400px' }">
                    <MakeOrderDashboard
                        :currentId="currentId"
                        moduleType="account" 
                        :makeOrderByPosData="makeOrderByPosData"
                    ></MakeOrderDashboard>
                </el-col>
                <el-col :span="17">
                    <el-tabs v-model="currentTradingDataTabName" type="border-card">
                        <el-tab-pane :label="`全部委托 ${showCurrentIdInTabName('orders')}`" name="orders">
                            <OrderRecord
                            moduleType="account" 
                            :noTitle="true"
                            :todayFinishPreSetting="true"
                            :accountType="accountType"
                            :currentId="currentId"
                            :kungfuData="orders"
                            :gatewayName="currentAccount.account_id"
                            :orderStat="orderStat"

                            @showHistory="handleShowHistory"
                            />   
                        </el-tab-pane>
                        <el-tab-pane :label="`未完成委托 ${showCurrentIdInTabName('unfinishedOrders')}`" name="unfinishedOrders">
                            <OrderRecord
                            moduleType="account" 
                            :noTitle="true"
                            :todayFinishPreSetting="false"
                            :accountType="accountType"
                            :currentId="currentId"
                            :kungfuData="orders"
                            :gatewayName="currentAccount.account_id"
                            :orderStat="orderStat"
                            @showHistory="handleShowHistory"
                            />   
                        </el-tab-pane>
                        <el-tab-pane :label="`成交记录 ${showCurrentIdInTabName('trades')}`" name="trades">
                            <TradeRecord
                            moduleType="account" 
                            :noTitle="true"
                            :currentId="currentId"
                            :kungfuData="trades"
                            :orderStat="orderStat"
                            @showHistory="handleShowHistory"
                            />
                        </el-tab-pane>
                    </el-tabs>
                </el-col>
            </el-row>

        </div>
    </MainContent>
</template>

<script>
import { mapState, mapGetters } from 'vuex'

import TdAccount from '@/components/Account/components/TdAccount';
import MdAccount from '@/components/Account//components/MdAccount';
import OrderRecord from '@/components/Base/tradingData/OrderRecord';
import TradeRecord from '@/components/Base/tradingData/TradeRecord';
import Pos from '@/components/Base/tradingData/Pos';
import MakeOrderDashboard from '@/components/Base/makeOrder/MakeOrderDashboard';
import MainContent from '@/components/Layout/MainContent';

import { buildTradingDataPipe } from '__io/kungfu/tradingData';
import { transformPositionByTickerByMerge, dealPos } from '__io/kungfu/watcher';
import accountStrategyMixins from '@/assets/js/mixins/accountStrategyMixins';

export default {
    name: 'account',

    mixins: [ accountStrategyMixins ],

    data() {
        this.tradingDataPipe = null;
        this.moduleType = 'account';

        return {
            orders: Object.freeze([]),
            trades: Object.freeze([]),
            positions: Object.freeze([]),
            positionsByTicker: Object.freeze([]),
            orderStat: Object.freeze({}),

            historyData: {},

            currentTradingDataTabName: "orders"
        }
    },

    components: {
        TdAccount, MdAccount, Pos,
        OrderRecord, TradeRecord,
        MakeOrderDashboard,
        MainContent
    },

    computed:{
        ...mapState({
            currentAccount: state => state.ACCOUNT.currentAccount, //选中的账户
            currentTicker: state => state.ACCOUNT.currentTicker,
            currentAccountTabName: state => state.ACCOUNT.currentAccountTabName,
            tdAccountSource: state => state.BASE.tdAccountSource || {},
        }),

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

        currentId() {
            return this.currentAccount.account_id || ''
        }
    },

    mounted(){
        this.tradingDataPipe = buildTradingDataPipe('account').subscribe(data => {
            this.dealTradingData(data);

            const positionsByTicker = data['positionsByTicker'];
            this.positionsByTicker = Object.freeze(transformPositionByTickerByMerge(positionsByTicker) || []);
            this.initSetCurrentTicker(this.positionsByTicker);

            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {});

            const assets = data['assets'];
            this.$store.dispatch('setAccountsAsset', Object.freeze(JSON.parse(JSON.stringify(assets))));
        })
    },

    methods: {

        handleAccountTabClick (tab, event) {
            this.$store.dispatch('setCurrentAccountTabName', tab.name)
        },

        handleShowHistory ({ date, data, type }) {
            this.$set(this.historyData, type, {
                date,
                data
            })
        },

        showCurrentIdInTabName (target) {
            return this.currentTradingDataTabName === target ? this.currentId : ''
        },

        setCurrentTicker (item) {
            this.$store.dispatch('setCurrentTicker', item)
        },

        initSetCurrentTicker (tickerList) {
            if (!this.currentTicker || !this.currentTicker.instrumentId) {
                if (tickerList.length) {
                    const tickerListSort = tickerList.slice(0).sort((a, b) => {
                        const result = a.instrument_id.localeCompare(b.instrument_id);
                        return result === 0 ? a.direction.localeCompare(b.direction) : result;
                    })

                    this.$store.dispatch('setCurrentTicker', dealPos(tickerListSort[0]))
                }
            }
        },

        dealTradingData (data) {
            if (this.historyData['order'] && ((this.historyData['order'] || {}).date)) {
                this.orders = Object.freeze(this.historyData['order'].data)
            } else {
                const orders = data['orders'][this.currentId];
                this.orders = Object.freeze(orders || []);
            }

            if (this.historyData['trade'] && ((this.historyData['trade'] || {}).date)) {
                this.trades = Object.freeze(this.historyData['trade'].data)
            } else {
                const trades = data['trades'][this.currentId];
                this.trades = Object.freeze(trades || []);
            }
      
            const positions = data['positions'][this.currentId];
            this.positions = Object.freeze(positions || []);

        },
    },

    destroyed(){
        this.tradingDataPipe && this.tradingDataPipe.unsubscribe();
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
}

.el-col{
    height: 100%;
}

</style>

