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
                        @startNanomsg="buildMinNanomsgListener"
                        :nanomsgBackData="minNanomsgListener"
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
                        @startNanomsg="buildOrderNanomsgListener"
                        :nanomsgBackData="orderNanomsgListener"
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
                    @startNanomsg="buildPosNanomsgListener"
                    :nanomsgBackData="posNanomsgListener"
                    />
                </el-row>
                
                <el-row style="height: 33.333%">
                    <TradeRecord
                    ref="trade-record"
                    :currentId="currentId"
                    moduleType="account" 
                    :getDataMethod="getAccountTrade"
                    @startNanomsg="buildTradeNanomsgListener"
                    :nanomsgBackData="tradeNanomsgListener"
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
import {sourceType} from '@/assets/config/accountConfig';
import {refreshGatewayNanomsg} from '@/io/nano/buildNmsg';
import * as msgType from '@/io/nano/msgType';
import * as ACCOUNT_API from '@/io/account';
import {debounce} from '@/assets/js/utils';


export default {
    name: 'account',
    data() {
        const t = this;
        t.oldOrderNanomsg = null; //保存之前的推送，可以随时根据存的数据关闭
        t.oldTradeNanomsg = null;
        t.oldPosNanomsg = null;
        t.oldMinNanomsg = null;
        return {
            orderNanomsgListener: null,
            tradeNanomsgListener: null,
            posNanomsgListener: null,
            minNanomsgListener: null
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
            return (t.currentAccount.account_id ? t.currentAccount.account_id.toAccountId() : '')
        }
    },

    destroyed() {
        const t = this
        if(t.oldOrderNanomsg) t.oldOrderNanomsg.close()
        if(t.oldTradeNanomsg) t.oldTradeNanomsg.close()
        if(t.oldPosNanomsg) t.oldPosNanomsg.close()
        if(t.oldMinNanomsg) t.oldMinNanomsg.close()
    },
 
    methods:{
        //获取委托推送信息
        buildOrderNanomsgListener() {
            const t = this
            const orderNanomsg = refreshGatewayNanomsg('td_'+ t.currentAccount.account_id, t.oldOrderNanomsg)
            if(!orderNanomsg) return;
            t.oldOrderNanomsg = orderNanomsg
            orderNanomsg.on('data', buf => {
                const orderData = JSON.parse(String(buf).replace(/\0/g,''))
                const {data, msg_type} = orderData
                if(msgType.order == msg_type) {
                    t.orderNanomsgListener = Object.freeze(data)
                }
            })
        },

        //获取成交推送信息
        buildTradeNanomsgListener() {
            const t = this
            const tradeNanomsg = refreshGatewayNanomsg('td_'+ t.currentAccount.account_id, t.oldTradeNanomsg)
            if(!tradeNanomsg) return;
            t.oldTradeNanomsg = tradeNanomsg
            tradeNanomsg.on('data', buf => {
                const tradeData = JSON.parse(String(buf).replace(/\0/g,''))
                const {data, msg_type} = tradeData
                if(msgType.trade == msg_type) {
                    t.tradeNanomsgListener = Object.freeze(data)
                }
            })
        },

        //获取持仓推送信息
        buildPosNanomsgListener() {
            const t = this
            const posNanomsg = refreshGatewayNanomsg('td_' + t.currentAccount.account_id, t.oldPosNanomsg)
            if(!posNanomsg) return;
            t.oldPosNanomsg = posNanomsg
            posNanomsg.on('data', buf => {
                const posData = JSON.parse(String(buf).replace(/\0/g,''))
                const {data, msg_type} = posData
                if(msgType.position == msg_type) {
                    t.posNanomsgListener = Object.freeze(data)
                }
            })
        },

        buildMinNanomsgListener() {
            const t = this
            const minNanomsg = refreshGatewayNanomsg('td_' + t.currentAccount.account_id, t.oldMinNanomsg)
            if(!minNanomsg) return;
            t.oldMinNanomsg = minNanomsg
            minNanomsg.on('data', buf => {
                const minData = JSON.parse(String(buf).replace(/\0/g,''))
                const {data, msg_type} = minData
                if(msgType.accountInfoByMin == msg_type) {
                    t.minNanomsgListener = Object.freeze(data)
                }
            })
        },

        restartAllNanoListener(){
            const t = this;
            t.$refs['current-order'] && t.orderNanomsgListener && t.orderNanomsgListener.close && t.orderNanomsgListener.close();
            t.$refs['current-order'] && t.$refs['current-order'].resetData() && t.$refs['current-order'].init();
            t.$refs['trade-record'] && t.tradeNanomsgListener && t.tradeNanomsgListener.close && t.tradeNanomsgListener.close();
            t.$refs['trade-record'] && t.$refs['trade-record'].resetData() && t.$refs['trade-record'].init();
            t.$refs['pos'] && t.posNanomsgListener && t.posNanomsgListener.close && t.posNanomsgListener.close();
            t.$refs['pos'] && t.$refs['pos'].resetData() && t.$refs['pos'].init();
            t.$refs['pnl'] && t.minNanomsgListener && t.minNanomsgListener.close && t.minNanomsgListener.close();
            t.$refs['pnl'] && t.$refs['pnl'].init();
        },

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

