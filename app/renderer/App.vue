<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<script>
import {deepClone} from '@/assets/js/utils';
import {mapState} from 'vuex';
import * as ACCOUNT_API from '@/io/account'
import * as BASE_API from '@/io/base'

import {BASE_DIR} from '__gConfig/pathConfig.js'
import {connectCalendarNanomsg} from '@/io/nano/buildNmsg'
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
        this.getMdTdState()
        this.getAccountsCash()
        this.getCalendarNanomsg()
        this.$store.dispatch('getStrategyList')
    },
    methods: {
        getMdTdState() {
            const t = this
            //先获取task的name,再去找他对应的状态
            t.$store.dispatch('getTasks').then(res => {
               if(!res) return
               t.$store.dispatch('setMdTdState', {})
                //分别获取状态
               res.forEach(item => {
                    const {name, task_type} = item
                    //type 是md,td
                    if(!(task_type === 'md' || task_type === 'td')) return
                    //查看是否存在该姓名的文件
                    if(existsSync(path.join(BASE_DIR, 'gateway', name))) {
                        t.$store.dispatch('buildGatewayNmsgListener', name)//监听账户信息，td、md状态，资金情况
                        ACCOUNT_API.getMdTdState(name).then(state => {
                            const len = (state || []).length;
                            const stateData = state[0]
                            if(stateData) t.$store.dispatch('setOneMdTdState', {
                                name, 
                                oneState: stateData
                            })
                        })
                    } 
                })
            })
        },

        //获取accounts的cash
        getAccountsCash() {
            const t = this
            let cashList = {}
            //从数据库中查找
            t.$store.dispatch('getAccountList').then(res => {
                if(!res) return
                const promises = res.map(item => {
                    const {account_id} = item
                    const accountId = account_id.toAccountId();
                    if(!existsSync(path.join(BASE_DIR,'accounts', accountId))) return false;
                    return ACCOUNT_API.getAccountAsset(accountId).then(cash => {
                        if(!cash || !cash.length) return false;
                        const cashData = cash[0];
                        return {
                            accountId: account_id,
                            cashData
                        }
                    })
                })

                Promise.all(promises).then(cashList => {
                    cashList.forEach(cash => cashList[cash.accountId] = cash.cashData)
                    t.$store.dispatch('setAccountsAsset', cashList)
                })
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