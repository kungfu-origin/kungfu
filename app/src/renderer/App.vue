<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<script>
import { deepClone } from '@/assets/js/utils';
import { mapState } from 'vuex';
import * as ACCOUNT_API from '@/io/db/account';
import * as BASE_API from '@/io/db/base';

import { BASE_DIR, buildAccountFolderPath } from '__gConfig/pathConfig.js'
import { connectCalendarNanomsg } from '@/io/nano/buildNmsg'
import * as msgType from '@/io/nano/msgType'
import { existsSync } from '__gUtils/fileUtils'
import path from 'path';



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
        this.getCalendarNanomsg()
        this.$store.dispatch('getStrategyList')
        this.$store.dispatch('getAccountList')
        .then(res => t.getAccountsCash())
    },
    methods: {
        buildMdTdStateNmsg(){},
        buildTradingDataNmsg(){},
    
        //获取accounts的cash
        getAccountsCash(accountList) {
            const t = this
            let cashList = {}
            //从数据库中查找
            if(!accountList || !accountList.length) return
            const promises = accountList.map(item => {
                const { account_id } = item
                if(!existsSync(buildAccountFolderPath(account_id))) return false;
                return ACCOUNT_API.getAccountAsset(account_id).then(cash => {
                    if(!cash || !cash.length) return false;
                    const cashData = cash[0];
                    return {
                        accountId: account_id,
                        cashData
                    }
                })
            })

            return Promise.all(promises).then(cashList => {
                cashList.forEach(cash => cashList[cash.accountId] = cash.cashData)
                t.$store.dispatch('setAccountsAsset', cashList)
            })
        },
        
        //获得交易日的推送
        getCalendarNanomsg() {
            const t = this
            //先主动获取
            t.$store.dispatch('getCalendar');
            const sub = connectCalendarNanomsg()
            if(!sub) return;
            sub.on('data', buf => {
                const data = JSON.parse(String(buf).replace(/\0/g,''))
                //监听交易日的变化
                if(msgType.calendar == data.msg_type) {
                t.$store.dispatch('setCalendar', data.data)
                }
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