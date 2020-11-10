<template>
    <MainContent>
        <div class="account-content">
            <el-col :span="monitTrades ? 10 : 14">
                <el-row style="height: 33.333%">
                    <el-col>
                       <TdAccount></TdAccount>
                    </el-col>
                </el-row>
                <el-row style="height: 33.333%" v-if="!monitOrders">
                    <el-col :span="monitTrades ? 12 : 14">
                        <MdAccount></MdAccount>
                    </el-col>
                    <el-col :span="monitTrades ? 12 : 10">
                        <Pnl 
                        :currentId="currentId" 
                        moduleType="account"
                        :minPnl="pnl"
                        :dailyPnl="dailyPnl"
                        />
                    </el-col>
                </el-row>
                <el-row :style="{ 'height': monitOrders ? '66.66%' : '33.333%' }">
                    <el-col>
                        <CurrentOrder
                        moduleType="account" 
                        :accountType="accountType"
                        v-model="monitOrders"
                        :currentId="currentId"
                        :kungfuData="orders"
                        :gatewayName="currentAccount.account_id"
                        :orderStat="orderStat"
                        @showHistory="handleShowHistory"
                        />   
                    </el-col>              
                </el-row>
            </el-col>

            <el-col :span="monitTrades ? 14 : 10">
                <el-row :style="{ 'height': monitTrades ? '33.33%' : '50%' }">
                    <Pos 
                    moduleType="account"
                    :currentId="currentId" 
                    :accountType="accountType"
                    :kungfuData="positions"
                    @showMakeOrderDashboard="handleShowOrCloseMakeOrderDashboard(true)"
                    @makeOrder="handleMakeOrderByPos"
                    />
                </el-row>
                
                <el-row :style="{ 'height': monitTrades ? '66.66%' : '50%' }">
                    <TradeRecord
                    moduleType="account" 
                    v-model="monitTrades"
                    :currentId="currentId"
                    :kungfuData="trades"
                    :orderStat="orderStat"
                    @showHistory="handleShowHistory"
                    />
                </el-row>
            </el-col>
        </div>
    </MainContent>
</template>

<script>
import {mapState, mapGetters} from 'vuex'

import TdAccount from './components/TdAccount';
import MdAccount from './components/MdAccount';
import CurrentOrder from '../Base/tradingData/CurrentOrder';
import TradeRecord from '../Base/tradingData/TradeRecord';
import Pos from '../Base/tradingData/Pos';
import Pnl from '../Base/tradingData/pnl/Index';
import MainContent from '@/components/Layout/MainContent';

import { buildTradingDataPipe } from '__io/kungfu/tradingData';
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
            pnl: Object.freeze([]),
            dailyPnl: Object.freeze([]),
            orderStat: Object.freeze({}),

            historyData: {},
            monitOrders: false,
            monitTrades: false,
        }
    },

    components: {
        TdAccount, MdAccount, Pnl, Pos,
        CurrentOrder, TradeRecord,
        MainContent
    },

    computed:{
        ...mapState({
            currentAccount: state => state.ACCOUNT.currentAccount, //选中的账户
            tdAccountSource: state => (state.BASE.tdAccountSource || {})
        }),

        //账户的类型，根据是哪个柜台的，可以判断是是期货还是股票还是证券
        accountType() {
            const t = this;
            const source_name = t.currentAccount.source_name
            if(!source_name) return
            return (t.tdAccountSource[source_name] || {}).typeName || ''
        },

        currentId() {
            const t = this;
            return t.currentAccount.account_id || ''
        }
    },

    mounted(){
        const t = this;
        t.tradingDataPipe = buildTradingDataPipe('account').subscribe(data => {
            if (this.historyData['order'] && ((this.historyData['order'] || {}).date)) {
                this.orders = Object.freeze(this.historyData['order'].data)
            } else {
                const orders = data['orders'][t.currentId];
                this.orders = Object.freeze(orders || []);
            }

            if (this.historyData['trade'] && ((this.historyData['trade'] || {}).date)) {
                this.trades = Object.freeze(this.historyData['trade'].data)
            } else {
                const trades = data['trades'][t.currentId];
                this.trades = Object.freeze(trades || []);
            }
      
            const positions = data['positions'][t.currentId];
            this.positions = Object.freeze(positions || []);
            const pnl = data['pnl'][t.currentId];
            this.pnl = Object.freeze(pnl || []);
            const dailyPnl = data['dailyPnl'][t.currentId];
            this.dailyPnl = Object.freeze(dailyPnl || []);
            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {});

            const assets = data['assets'];
            this.$store.dispatch('setAccountsAsset', Object.freeze(JSON.parse(JSON.stringify(assets))));
        })
    },

    methods: {

       handleShowHistory ({ date, data, type }) {
            this.$set(this.historyData, type, {
                date,
                data
            })
        }
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

.account-content > .el-row{
    height: 50%;
}
.el-col{
    height: 100%;
}
.el-row > .tr-dashboard{
    padding-right: 0px;
}
.el-col > .el-row:last-child .tr-dashboard{
    padding-bottom: 0px;
}

// .account-content > .el-row:last-child .tr-dashboard{
//     padding-bottom: 0px;
// }


</style>

