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
                        ref="pnl"
                        :currentId="currentId" 
                        moduleType="account"
                        :minMethod="getAccountPnlMin"
                        :dayMethod="getAccountPnlDay"
                        :nanomsgBackData="minPnlFromNmsg"
                        />
                    </el-col>
                </el-row>
                <el-row style="height: 33.333%">
                    <el-col>
                        <CurrentOrder
                        ref="current-order"
                        :currentId="currentId"
                        moduleType="account" 
                        :gatewayName="`td_${currentAccount.account_id}`"
                        :getDataMethod="getAccountOrder" 
                        :nanomsgBackData="ordersFromNmsg"
                        />   
                    </el-col>              
                </el-row>
            </el-col>

            <el-col :span="10">
                <el-row style="height: 66.666%">
                    <Pos 
                    ref="pos"
                    :currentId="currentId" 
                    moduleType="account"
                    :accountType="accountType"
                    :getDataMethod="getAccountPos"
                    :nanomsgBackData="posFromNmsg"
                    />
                </el-row>
                
                <el-row style="height: 33.333%">
                    <TradeRecord
                    ref="trade-record"
                    :currentId="currentId"
                    moduleType="account" 
                    :getDataMethod="getAccountTrade"
                    :nanomsgBackData="tradesFromNmsg"
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
import CurrentOrder from '../Base/CurrentOrder';
import TradeRecord from '../Base/TradeRecord';
import Pos from '../Base/Pos';
import Pnl from '../Base/pnl/Pnl';
import { sourceType } from '@/assets/config/accountConfig';
import * as ACCOUNT_API from '@/io/db/account';
import { debounce } from '@/assets/js/utils';
import { buildTradingDataPipe, buildCashPipe } from '@/io/nano/nanoSub';
import * as MSG_TYPE from '@/io/nano/msgType';


export default {
    name: 'account',
    data() {
        const t = this;
        return {
            ordersFromNmsg: null,
            tradesFromNmsg: null,
            posFromNmsg: null,
            minPnlFromNmsg: null
        }
    },

    components: {
        TdAccount, MdAccount, Pnl, Pos,
        CurrentOrder, TradeRecord
    },

    computed:{
        ...mapState({
            currentAccount: state => state.ACCOUNT.currentAccount, //选中的账户
        }),

        //账户的类型，根据是哪个柜台的，可以判断是是期货还是股票还是证券
        accountType() {
            const source_name = this.currentAccount.source_name
            if(!source_name) return
            return sourceType[source_name].typeName
        },

        currentId() {
            const t = this;
            return t.currentAccount.account_id || ''
        }
    },

    mounted(){
        const t = this;
        buildTradingDataPipe().subscribe(d => {
            const msgType = d.msg_type;
            const tradingData = d.data;
            const accountId = tradingData.account_id || '';
            const currentId = t.currentId.toAccountId();
            switch (msgType) {
                case MSG_TYPE.order:
                    if(accountId !== currentId) return;
                    t.ordersFromNmsg = Object.freeze(tradingData);
                    break
                case MSG_TYPE.trade:
                    if(accountId !== currentId) return;
                    t.tradesFromNmsg = Object.freeze(tradingData);
                    break
                case MSG_TYPE.position:
                    if(accountId !== currentId) return;
                    t.posFromNmsg = Object.freeze(tradingData);
                    break
            }
        })

        buildCashPipe().subscribe(({ data }) => {
            const { account_id, source_id } = data;
            const accountId = `${source_id}_${account_id}`;  
            t.$store.dispatch('setAccountAssetById', { accountId, accountAsset: Object.freeze(data) })
        })
    },
 
    methods:{
        getAccountPos: ACCOUNT_API.getAccountPos,
        getAccountOrder: ACCOUNT_API.getAccountOrder,
        getAccountTrade: ACCOUNT_API.getAccountTrade,
        getAccountPnlMin: ACCOUNT_API.getAccountPnlMin,
        getAccountPnlDay: ACCOUNT_API.getAccountPnlDay,

    }
}
</script>

<style lang="scss" scoped>
@import '@/assets/scss/skin.scss';
.account-content{
    height: 100%;
}

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

