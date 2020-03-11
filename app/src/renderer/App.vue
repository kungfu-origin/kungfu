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
import { buildKungfuGlobalDataPipe } from '__io/kungfu/index';
import { deleteProcess } from '__gUtils/processUtils';

export default {
    name: 'app',
    data() {
        this.kungfuGloablDataObserver = null;
        return {
            globalSettingDialogVisiblity: false
        }
    },

    components: {
        GlobalSettingDialog
    },

    mounted(){
        this.removeLoadingMask();
        this.removeKeyDownEvent();
        //ipc event
        this.bindMainProcessEvent();

        this.$store.dispatch('getTdMdList');
        this.$store.dispatch('getStrategyList');
        this.$store.dispatch('getAccountSourceConfig');
        this.$store.dispatch('getKungfuConfig');
        this.kungfuGloablDataObserver = this.subKungfuGlobalData();

    },

    destroyed() {
        this.kungfuGloablDataObserver.unsubscribe();
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
                    if(key.includes('td') || key.includes('md')) {
                        t.$store.dispatch('deleteOneMdTdState', key)
                    }
                }
            })
        }
    },

    methods: {   
        removeLoadingMask () {
            //code 模块，暂时不做成单页， 需要用这种方法来避免code模块出现问题
            if(window.location.hash.includes('code')) return 

            //remove loading mask
            if(document.getElementById('loading')) document.getElementById('loading').remove();
        },

        subKungfuGlobalData () {
            return buildKungfuGlobalDataPipe().subscribe(data => {
                const tradingDay = data.tradingDay;
                this.$store.dispatch('setTradingDay', tradingDay);
                data.gatewayStates.forEach(gatewayState => {
                    this.$store.dispatch('setOneMdTdState', {
                        id: gatewayState.processId,
                        stateData: gatewayState
                    })
                })
            })
        },

        removeKeyDownEvent () {
            //解除回车带来的一些不好的影响
            //比如页面重新刷新的问题
            if (this.$router.history.current.name === 'kungfuCodeEditor') return      
            document.body.addEventListener('keydown', (event) => {
                if(event.keyCode == 13) {
                    event.preventDefault()
                }
            })
        },

        bindMainProcessEvent () {
            const t = this;
            ipcRenderer.removeAllListeners('main-process-messages')
            ipcRenderer.on('main-process-messages', (event, args) => {
                switch (args) {
                    case 'open-setting-dialog':
                        t.globalSettingDialogVisiblity = true;
                        break
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