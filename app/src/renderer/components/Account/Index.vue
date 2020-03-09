<template>
    <main-content>
        <div class="account-content">
            <el-col :span="14">
                <el-row style="height: 33.333%">
                    <el-col>
                       <TdAccount/>
                    </el-col>
                </el-row>
                <el-row style="height: 33.333%">
                    <el-col :span="14">
                        <MdAccount></MdAccount>
                    </el-col>
                    <el-col :span="10">
                        <Pnl 
                        :currentId="currentId" 
                        moduleType="account"
                        :dayMethod="getAccountPnlDay"
                        :kungfuData="pnl"
                        />
                    </el-col>
                </el-row>
                <el-row style="height: 33.333%">
                    <el-col>
                        <CurrentOrder
                        moduleType="account" 
                        :currentId="currentId"
                        :tradingDay="tradingDay"
                        :kungfuData="orders"
                        :gatewayName="`td_${currentAccount.account_id}`"
                        :orderStat="orderStat"
                        />   
                    </el-col>              
                </el-row>
            </el-col>

            <el-col :span="10">
                <el-row style="height: 50%">
                    <Pos 
                    moduleType="account"
                    :currentId="currentId" 
                    :accountType="accountType"
                    :kungfuData="positions"
                    />
                </el-row>
                
                <el-row style="height: 50%">
                    <TradeRecord
                    moduleType="account" 
                    :currentId="currentId"
                    :tradingDay="tradingDay"
                    :kungfuData="trades"
                    />
                </el-row>
            </el-col>
        </div>
    </main-content>
</template>

<script>
import {mapState, mapGetters} from 'vuex'

import TdAccount from './components/TdAccount';
import MdAccount from './components/MdAccount';
import CurrentOrder from '../Base/tradingData/CurrentOrder';
import TradeRecord from '../Base/tradingData/TradeRecord';
import Pos from '../Base/tradingData/Pos';
import Pnl from '../Base/tradingData/pnl/Index';

import { buildTradingDataPipe } from '__io/kungfu/index';

export default {
    name: 'account',
    data() {
        const t = this;
        this.tradingDataPipe = null;
        return {
            orders: Object.freeze([]),
            trades: Object.freeze([]),
            positions: Object.freeze([]),
            pnl: Object.freeze([]),
            orderStat: Object.freeze({}),
        }
    },

    components: {
        TdAccount, MdAccount, Pnl, Pos,
        CurrentOrder, TradeRecord
    },

    computed:{
        ...mapState({
            tradingDay: state => state.BASE.tradingDay, //日期信息，包含交易日
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
            const orders = data['orders'][t.currentId];
            this.orders = Object.freeze(orders || []);
            const trades = data['trades'][t.currentId];
            this.trades = Object.freeze(trades || []);
            const positions = data['positions'][t.currentId];
            this.positions = Object.freeze(positions || []);
            const assets = data['assets'];
            this.$store.dispatch('setAccountsAsset', Object.freeze(assets));
            const pnl = data['pnl'][t.currentId];
            this.pnl = Object.freeze(pnl || []);
            const orderStat = data['orderStat'];
            this.orderStat = Object.freeze(orderStat || {})
        })
    },

    destroyed(){
        const t = this;
        t.tradingDataPipe && t.tradingDataPipe.unsubscribe();
    },
 
    methods:{
        getAccountPnlDay: () => { return Promise.resolve([]) }
    }
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

