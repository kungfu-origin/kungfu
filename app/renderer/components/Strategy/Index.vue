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
                                @startNanomsg="buildMinNanomsgListener"
                                :nanomsgBackData="minNanomsgBack"
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
                            @startNanomsg="buildPosNanomsgListener"
                            :nanomsgBackData="posNanomsgBack"                   
                            ></Pos>
                    </el-row>
                    <el-row  style="height: 33.333%">
                            <CurrentOrder
                            ref="current-order"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :getDataMethod="getStrategyOrder"
                            @startNanomsg="buildOrderNanomsgListener"
                            :nanomsgBackData="orderNanomsgBack"
                            ></CurrentOrder>                      
                    </el-row>
                    <el-row style="height: 33.333%">
                            <TradeRecord 
                            ref="trade-record"
                            :currentId="strategyId"
                            moduleType="strategy"
                            :getDataMethod="getStrategyTrade"
                            @startNanomsg="buildTradeNanomsgListener"
                            :nanomsgBackData="tradeNanomsgBack"
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
import {mapState, mapGetters} from 'vuex';
import * as STRATEGY_API from '@/io/strategy'
import {refreshStrategyNanomsg, refreshGatewayNanomsg} from '@/io/nano/buildNmsg'
import * as msgType from '@/io/nano/msgType'


export default {
    data(){
        const t = this;
        t.oldOrderNanomsgListener = {}; //记住nanomsg,以便之后清除
        t.oldTradeNanomsgListener = {}; // 监听trade变化 
        t.oldPosNanomsgListener = null; //监听pos变化
        t.oldMinNanomsgListener = null;  //监听分钟线变化
        t.oldStrAccountNanomsgListener = null; //监听策略使用账户变化
        return {
            orderNanomsgBack: null, //传入order组件中的nanomsg推送的数据
            posNanomsgBack: null, //传入pos组件中的nanomsg推送的数据
            tradeNanomsgBack: null, //传入trades组件中的nanomsg推送的数据
            minNanomsgBack: null, //传入min组件中的nanomsg推送的数据
        }
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

    destroyed() {
        const t = this
        t.closeNanomsg(t.oldOrderNanomsgListener)
        t.closeNanomsg(t.oldTradeNanomsgListener)
        t.oldPosNanomsgListener && t.oldPosNanomsgListener.close && t.oldPosNanomsgListener.close();
        t.oldMinNanomsgListener && t.oldMinNanomsgListener.close && t.oldMinNanomsgListener.close();
        t.oldStrAccountNanomsgListener && t.oldStrAccountNanomsgListener.close && t.oldStrAccountNanomsgListener.close();
    },

    watch: {
        strategyId(val) {
            const t = this;
            if(!val) return
            //订阅账户变更
            t.buildStrategyAccountsNanomsgListener()
        }
    },

    mounted(){
        const t = this;
        if(t.strategyId) t.buildStrategyAccountsNanomsgListener()
    },

    methods:{
        //获取委托推送信息
        buildOrderNanomsgListener() {
            const t = this
            //获取推送前将之前的订阅关闭
            t.getStrategyAccounts().then((strategyAccounts) => {
                strategyAccounts.forEach(item => {
                    const gatewayName = `td_${item.source}_${item.account_id}`
                    const gateWayNanomsgListener = refreshGatewayNanomsg(gatewayName, t.oldOrderNanomsgListener[gatewayName] || undefined)
                    if(!gateWayNanomsgListener) return;
                    t.oldOrderNanomsgListener[gatewayName] = gateWayNanomsgListener
                    t.oldOrderNanomsgListener[gatewayName].on('data', buf => {
                        const orderData = JSON.parse(String(buf).replace(/\0/g,''))
                        const {data, msg_type} = orderData
                        if(msgType.order == msg_type && data.client_id === t.strategyId) {
                            t.orderNanomsgBack = Object.freeze(data)
                        }
                    })
                })
            })
        },

        //获取成交推送信息
        buildTradeNanomsgListener() {
            const t = this
            //获取推送前将之前的订阅关闭
            t.getStrategyAccounts().then((strategyAccounts) => {
                strategyAccounts.forEach(item => {
                    const gatewayName = `td_${item.source}_${item.account_id}`
                    const gateWayNanomsgListener = refreshGatewayNanomsg(gatewayName, t.oldTradeNanomsgListener[gatewayName] || undefined)
                    if(!gateWayNanomsgListener) return;
                    t.oldTradeNanomsgListener[gatewayName] = gateWayNanomsgListener                    
                    t.oldTradeNanomsgListener[gatewayName].on('data', buf => {
                        const tradeData = JSON.parse(String(buf).replace(/\0/g,''))
                        const {data, msg_type} = tradeData
                        if(msgType.trade == msg_type && data.client_id === t.strategyId) {
                            t.tradeNanomsgBack = Object.freeze(data)
                        }
                    })
                })
            })
        },

        //获取持仓
        buildPosNanomsgListener() {
            const t = this
            const posNanomsg = refreshStrategyNanomsg(t.strategyId, t.oldPosNanomsgListener)
            if(!posNanomsg) return;
            t.oldPosNanomsgListener = posNanomsg
            posNanomsg.on('data', buf => {
                const posData = JSON.parse(String(buf).replace(/\0/g,''))
                const {data, msg_type} = posData
                if(msgType.position == msg_type && data.client_id === t.strategyId) {
                    t.posNanomsgBack = Object.freeze(data)
                }
            })
        },

        //获取pnl min线
        buildMinNanomsgListener() {
            const t = this
            const minNanomsg = refreshStrategyNanomsg(t.strategyId, t.oldMinNanomsgListener)
            if(!minNanomsg) return;
            t.oldMinNanomsgListener = minNanomsg
            minNanomsg.on('data', buf => {
                const minData = JSON.parse(String(buf).replace(/\0/g,''))
                const {data, msg_type} = minData
                if(msgType.portfolioByMin == msg_type) {
                    t.minNanomsgBack = Object.freeze(data)
                }
            })
        },

        //当策略下的账户有变化的时候会推送数据
        buildStrategyAccountsNanomsgListener() {
            const t = this
            const stratAccountNanomsg = refreshStrategyNanomsg(t.strategyId, t.oldStrAccountNanomsgListener)
            if(!stratAccountNanomsg) return;
            t.oldStrAccountNanomsgListener = stratAccountNanomsg
            stratAccountNanomsg.on('data',buf => {
                const accountData = JSON.parse(String(buf).replace(/\0/g,''))
                const {data, msg_type} = accountData
                if(msgType.strategyUsedAccountUpdate == msg_type) {
                    t.$refs['current-order'] && t.closeNanomsg(t.oldOrderNanomsgListener) && t.$refs['current-order'].resetData() && t.$refs['current-order'].init();
                    t.$refs['trade-record'] && t.closeNanomsg(t.oldTradeNanomsgListener) && t.$refs['trade-record'].resetData() && t.$refs['trade-record'].init();
                }
            })
        },

        //close trade/order nano msg , because they are built by diff accounts
        closeNanomsg(nanomsg) {
            const nanomsgKeys = Object.keys(nanomsg || {});
            if(nanomsgKeys.length) {
                nanomsgKeys.map(key => {
                    nanomsg[key] && nanomsg[key].close();
                })
            }
            return true;
        },

        //获取策略下的账户
        getStrategyAccounts() {
            const t = this
            return STRATEGY_API.getStrategyAccounts(t.strategyId)
        },

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
