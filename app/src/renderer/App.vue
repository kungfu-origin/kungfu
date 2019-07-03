<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<script>
import path from 'path';
import { mapState } from 'vuex';

import { KF_HOME, LIVE_TRADING_DB_DIR } from '__gConfig/pathConfig.js';
import { existsSync } from '__gUtils/fileUtils';
import { deepClone } from '@/assets/js/utils';
import * as ACCOUNT_API from '@/io/db/account';
import * as BASE_API from '@/io/db/base';
import { connectCalendarNanomsg } from '@/io/nano/buildNmsg'
import * as MSG_TYPE from '@/io/nano/msgType'
import { buildGatewayStatePipe } from '@/io/nano/nanoSub'; 



export default {
    name: 'app',
    data() {
        return {
        }
    },
    mounted(){
        if(document.getElementById('loading')){
            document.getElementById('loading').remove()
        };
    },
    created() {
        const t = this;
        this.subGatewayState();
        this.$store.dispatch('getStrategyList')
        this.$store.dispatch('getAccountList')
        .then(accountList => t.getAccountsCash(accountList))
        this.getCalendarNanomsg();
    },
    methods: {   
        subGatewayState() {
            const t = this;
            buildGatewayStatePipe().subscribe(data => {
                t.$store.dispatch('setOneMdTdState', {
                    id: data[0],
                    stateData: data[1]
                })
            })
        },
        
        //获取accounts的cash
        getAccountsCash(accountList) {
            const t = this
            //从数据库中查找
            if(!accountList || !accountList.length) return
            const promises = accountList.map(({ account_id }) => {
                if(!existsSync(LIVE_TRADING_DB_DIR)) return false;
                return ACCOUNT_API.getAccountAsset(account_id).then(cash => {
                    if(!cash || !cash.length) return false;
                    return { accountId: account_id, cashData: cash[0] }
                })
            })
            return Promise.all(promises).then(cashList => {
                const cashData = {} 
                cashList.forEach(cash => cashData[cash.accountId] = cash.cashData)
                t.$store.dispatch('setAccountsAsset', cashData)
            })
        },
        
        //获得交易日的推送
        getCalendarNanomsg() {
            const t = this
            //先主动获取
            t.$store.dispatch('getCalendar');
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