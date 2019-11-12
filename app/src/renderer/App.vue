<template>
    <div id="app">
        <router-view></router-view>
        <GlobalSettingDialog
            v-if="globalSettingDialogVisiblity"
            :visible.sync="globalSettingDialogVisiblity"
        >
        </GlobalSettingDialog>
    </div>
</template>
<script>
import path from 'path';
import { mapState } from 'vuex';
import moment from 'moment';
import { ipcRenderer } from 'electron';

import GlobalSettingDialog from './components/Base/GlobalSettingDialog';

import { KF_HOME, LIVE_TRADING_DB_DIR } from '__gConfig/pathConfig';
import { existsSync } from '__gUtils/fileUtils';
import { deepClone, delayMiliSeconds, debounce } from '__gUtils/busiUtils';
import { getAccountAsset } from '__io/db/account';
import { connectCalendarNanomsg } from '__io/nano/buildNmsg'
import * as MSG_TYPE from '__io/nano/msgType'
import { buildGatewayStatePipe, buildCashPipe, buildTradingDayPipe } from '__io/nano/nanoSub'; 
import { deleteProcess } from '__gUtils/processUtils';
import { getAccountSource } from '__gConfig/accountConfig';
import { nanoReqGatewayState, nanoReqCash, nanoReqCalendar } from '__io/nano/nanoReq';

export default {
    name: 'app',
    data() {
        this.gatewayStatePipe = null;
        this.cashPipe = null;
        this.tradingDayPipe = null;
        return {
            globalSettingDialogVisiblity: false
        }
    },

    components: {
        GlobalSettingDialog
    },

    mounted(){
        const t = this;
        //code 模块，暂时不做成单页， 需要用这种方法来避免code模块出现问题
        if(window.location.hash.indexOf('code') !== -1) return 

        //remove loading mask
        if(document.getElementById('loading')) document.getElementById('loading').remove();
        
        //解除回车带来的一些不好的影响
        //比如页面重新刷新的问题
        document.body.addEventListener('keydown', (event) => {
            if(event.keyCode == 13) {
                event.preventDefault()
            }
        })

        //ipc event
        ipcRenderer.removeAllListeners('main-process-messages')
        t.bindMainProcessEvent()

    },

    created() {
        const t = this;
        t.$store.dispatch('getAccountSourceConfig')
        t.$store.dispatch('getStrategyList')
        t.$store.dispatch('getAccountList')
            .then(accountList => t.getAccountsCash(accountList))

        t.subGatewayState();
        t.subAccountCash();
        t.subTradingDay();
      
        t.reqCalendar();
        t.reqCash();
        t.reqGatewayState();

        t.$store.dispatch('getKungfuConfig')
    },

    destroyed() {
        const t = this;
        t.gatewayStatePipe.unsubscribe();
        t.cashPipe.unsubscribe();
        t.tradingDayPipe.unsubscribe();

        ipcRenderer.removeAllListeners('main-process-messages')        
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus
        })
    },

    watch: {
        //reset state of td/md every time close
        processStatus: function(status){
            const t = this;
            Object.keys(status || {}).forEach(key => {
                const s = status[key]
                if(s !== 'online') {
                    if(key.indexOf('td') !== -1 || key.indexOf('md') !== -1) {
                        t.$store.dispatch('deleteOneMdTdState', key)
                    }
                }
            })
        }
    },

    methods: {   
        subGatewayState() {
            const t = this;
            t.gatewayStatePipe = buildGatewayStatePipe().subscribe(data => {
                const processId = data[0];
                const stateData = data[1];
                //if state is 2 means disconnect, kill process, delay 3s; 
                if(+stateData.state === 5) {
                    delayMiliSeconds(1000)
                    .then(() => deleteProcess(processId))
                } else { 
                    // console.log('[GATEWAY STATE] sub', processId, stateData)
                    t.$store.dispatch('setOneMdTdState', {
                        id: processId,
                        stateData: stateData
                    })
                }
            })
        },

        subAccountCash() {
            const t = this;
            t.cashPipe = buildCashPipe().subscribe(({ data }) => {
                const { account_id, source_id, ledger_category } = data;
                const accountId = `${source_id}_${account_id}`;                  
                if(ledger_category !== 0) return;
                // console.log('[CASH] sub',  accountId, data)
                t.$store.dispatch('setAccountAssetById', { accountId, accountsAsset: Object.freeze({
                    ...data,
                    nano: true
                })})
            })
        },

        subTradingDay() {
            const t = this;
            //sub 交易日
            t.tradingDayPipe = buildTradingDayPipe().subscribe(d => {
                const calendar = d.data;
                if(calendar && calendar.trading_day) {
                    const tradingDay = moment(calendar.trading_day).format('YYYYMMDD');
                    // console.log('[TRADING DAY] sub', tradingDay)
                    t.$store.dispatch('setTradingDay', tradingDay);
                }
            })
        },
        
        //获取accounts的cash
        getAccountsCash(accountList) {
            const t = this
            //从数据库中查找
            if(!accountList || !accountList.length) return
            getAccountAsset().then(cashList => {
                const cashData = [{}, ...cashList].reduce((cash, curr) => {
                    cash[`${curr.source_id}_${curr.account_id}`] = curr
                    return cash
                })
                t.$store.dispatch('setAccountsAsset', cashData)
            })
        },

        bindMainProcessEvent() {
            const t = this;
            ipcRenderer.on('main-process-messages', (event, args) => {
                switch (args) {
                    case 'open-setting-dialog':
                        t.globalSettingDialogVisiblity = true;
                        break
                }
            })
        },
        
        //获得交易日的推送
        reqCalendar() {
            const t = this
            //先主动获取
            delayMiliSeconds(3000)//需要等ledger起来
            .then(() => nanoReqCalendar())
            .then(calendar => {
                if(calendar && calendar.trading_day) {
                    const tradingDay = moment(calendar.trading_day).format('YYYYMMDD');
                    t.$store.dispatch('setTradingDay', tradingDay);
                }
            })
            .catch(err => console.error(err))
        },

        //获取gatewayState（req后会从subGatewayState中获取）
        reqGatewayState(){
            delayMiliSeconds(3000)//需要等ledger起来
            .then(() => nanoReqGatewayState())
        },

        //获取资金信息
        reqCash() {
            delayMiliSeconds(3000)//需要等ledger起来
            .then(() => nanoReqCash())
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