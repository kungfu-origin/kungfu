<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<script>
import path from 'path';
import { mapState } from 'vuex';

import { KF_HOME, LIVE_TRADING_DB_DIR } from '__gConfig/pathConfig';
import { existsSync } from '__gUtils/fileUtils';
import { deepClone } from '__gUtils/busiUtils';
import * as ACCOUNT_API from '__io/db/account';
import * as BASE_API from '__io/db/base';
import { connectCalendarNanomsg } from '__io/nano/buildNmsg'
import * as MSG_TYPE from '__io/nano/msgType'
import { buildGatewayStatePipe, buildCashPipe } from '__io/nano/nanoSub'; 
import { deleteProcess } from '__gUtils/processUtils';
import { getAccountSource } from '__gConfig/accountConfig';


export default {
    name: 'app',
    data() {
        this.gatewayStatePipe = null;
        this.cashPipe = null;
        return {}
    },

    mounted(){
        if(document.getElementById('loading')) document.getElementById('loading').remove();
    },

    created() {
        const t = this;
        this.getAccountSource();
        this.$store.dispatch('getStrategyList')
        this.$store.dispatch('getAccountList')
        .then(accountList => t.getAccountsCash(accountList))

        this.subGatewayState();
        this.subAccountCash();
      
        this.reqCalendar();
    },

    destroyed() {
        const t = this;
        t.gatewayStatePipe.unsubscribe();
        t.cashPipe.unsubscribe();
    },

    methods: {   
        subGatewayState() {
            const t = this;
            t.gatewayStatePipe = buildGatewayStatePipe().subscribe(data => {
                const processId = data[0];
                const stateData = data[1];
                //if state is 2 means disconnect, kill process; 
                if(stateData.state === 2) deleteProcess(processId);
                t.$store.dispatch('setOneMdTdState', {
                    id: processId,
                    stateData: stateData
                })
            })
        },

        subAccountCash() {
            const t = this;
            t.cashPipe = buildCashPipe().subscribe(({ data }) => {
                const { account_id, source_id, ledger_category } = data;
                const accountId = `${source_id}_${account_id}`;                  
                if(ledger_category !== 0) return;
                t.$store.dispatch('setAccountAssetById', { accountId, accountAsset: Object.freeze(data) })
            })
        },
        
        //获取accounts的cash
        getAccountsCash(accountList) {
            const t = this
            //从数据库中查找
            if(!accountList || !accountList.length) return
            ACCOUNT_API.getAccountAsset().then(cashList => {
                const cashData = {} 
                cashList.forEach(cash => cashData[`${cash.source_id}_${cash.account_id}`] = cash)
                t.$store.dispatch('setAccountsAsset', cashData)
            })
        },
        
        //获得交易日的推送
        reqCalendar() {
            const t = this
            //先主动获取
            t.$store.dispatch('reqCalendar')
        },

        //获取柜台信息
        getAccountSource() {
            const t = this;
            getAccountSource().then(res => {
                t.$store.dispatch('setAccountSource', res)
            })
        }
    }
}
</script>

<style lang="scss">
@import './assets/scss/base.scss';
 #app{
   height: 100%;
   background: $login-bg;
 }
</style>