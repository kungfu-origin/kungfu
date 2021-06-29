<template>
    <div id="app">
        <router-view></router-view>

        <GlobalSettingDialog
            v-if="globalSettingDialogVisiblity"
            :visible.sync="globalSettingDialogVisiblity"
        >
        </GlobalSettingDialog>

        <SystemPrepareDialog
        title="系统提示"
        :visible="watcherLoading"
        :status="[
            { key: 'archive', status: loadingData.archive },
            { key: 'watcher', status: loadingData.watcher },
            { key: 'daemon', status: loadingData.daemon },
        ]"
        :tips="{
            'archive': { done: '功夫归档完成 ✓',     loading: '功夫归档中...' },
            'watcher': { done: '功夫环境准备完成 ✓',  loading: '功夫环境准备中...' },
            'daemon':  { done: '功夫数据通信已建立 ✓', loading: '等待功夫数据通信建立...' },
        }"
        ></SystemPrepareDialog>

        <SystemPrepareDialog
        title="系统提示"
        :visible="recordBeforeQuitLoading"
        :status="[
            { key: 'record', status: recordBeforeQuitLoading },
        ]"
        :tips="{
            'record': { done: '保存数据完成 ✓',     loading: '退出前保存数据中，请勿关闭...' },
        }"
        ></SystemPrepareDialog>

        <date-picker-dialog 
        @confirm="handleConfirmDateRangeForExportAllTradingData"
        v-if="exportAllTradingDataByDateDateRangeDialogVisiblity"
        :visible.sync="exportAllTradingDataByDateDateRangeDialogVisiblity"   
        :loading="exportAllTradingDataByDateDateRangeDialogLoading" 
        ></date-picker-dialog>

        <AuthDialog :visible.sync="authingGuardVisiblity"></AuthDialog>

    </div>
</template>
<script>

import { mapState } from 'vuex';

import GlobalSettingDialog from '@/components/Base/GlobalSettingDialog';
import DatePickerDialog from '@/components/Base/DatePickerDialog';
import SystemPrepareDialog from '@/components/Layout/components/SystemPrepareDialog';
import AuthDialog from '@/components/Layout/components/AuthingDialog';

import { buildMarketDataPipeByDaemon, buildTradingDataAccountPipeByDaemon, buildKungfuGlobalDataPipeByDaemon } from '@/ipcMsg/daemon';
import { buildGatewayStatePipe } from '__io/kungfu/tradingData';
import { watcher } from '__io/kungfu/watcher';

import ipcListenerMixin from '@/ipcMsg/ipcListenerMixin';
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';
import workersMixin from '@/workers/workersMixin';



export default {
    name: 'app',

    mixins: [ ipcListenerMixin, tickerSetMixin, workersMixin ],

    data() {
        this.kungfuGloablDataObserver = null;
        
        return {

            globalSettingDialogVisiblity: false,
            authingGuardVisiblity: false,
          
            loadingData: {
                archive: false,
                watcher: false,
                daemon: false,
            }
        }
    },

    components: {
        DatePickerDialog,
        GlobalSettingDialog,
        SystemPrepareDialog,
        AuthDialog
    },

    mounted(){

        //打开应用后报错，比如vs2019依赖, 在app启动后不显示
        window.AFTER_APP_MOUNTED = true;

        this.removeLoadingMask();
        this.removeKeyDownEvent();

        this.$store.dispatch('getTdMdList');
        this.$store.dispatch('getStrategyList');
        this.$store.dispatch('getAccountSourceConfig');
        this.$store.dispatch('getKungfuConfig');
        this.$store.dispatch('getExtensionConfigs')

        this.bindKungfuGlobalDataListener();
        this.bindTradingDataListener();
        this.bindQuotesListener();
        this.bindBusEvent();
        this.bindMdTdStateChangeEvent();

        this.getWatcherStatus();
    },

    beforeDestroy() {
        this.$bus.$off('mdTdStateReady');
        this.kungfuGatewayStateObserver && this.kungfuGatewayStateObserver.unsubscribe()
        this.kungfuGloablDataObserver && this.kungfuGloablDataObserver.unsubscribe();
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus || {},
            daemonIsLive: state => state.BASE.daemonIsLive || false,
        }),

        watcherLoading () {
            const { archive, daemon } = this.loadingData;
            return !(archive && this.loadingData.watcher && daemon)
        }
    },

    watch: {
        //reset state of td/md every time close
        processStatus: function(status){
            Object.keys(status || {}).forEach(key => {
                const s = status[key]
                if(s !== 'online') {
                    if(key.includes('td') || key.includes('md')) {
                        this.$store.dispatch('deleteOneMdTdState', key)
                    }
                }
            })
        }
    },

    methods: {
        bindQuotesListener () {
            buildMarketDataPipeByDaemon().subscribe(data => {
                this.$store.dispatch('setQuotes', Object.freeze(Object.values(data)))   
            })
        },

        bindTradingDataListener () {
            buildTradingDataAccountPipeByDaemon().subscribe(data => {
                const assets = data['assets'];
                this.$store.dispatch('setAccountsAsset', Object.freeze(assets));
            })
        },

        bindKungfuGlobalDataListener () {
            this.kungfuGloablDataObserver = buildKungfuGlobalDataPipeByDaemon().subscribe(data => {
                //非常有必要，需要确保daemon进程watcherisLive
                const daemonIsLive = data["daemonIsLive"] || false;
                this.$store.dispatch("setDaemonIsLive", daemonIsLive)
                
            })

            this.kungfuGatewayStateObserver = buildGatewayStatePipe().subscribe(data => {
                const gatewayStates = data["gatewayStates"] || [];
                this.dealGatewayStates(gatewayStates);

            })
        },

        bindBusEvent () {
            this.$bus.$on('open-global-setting', () => {
                this.globalSettingDialogVisiblity = true;
            })

            this.$bus.$on('open-authing-guard', () => {
                this.authingGuardVisiblity = true;
            })
        },

        removeLoadingMask () {
            if(document.getElementById('loading')) document.getElementById('loading').remove();
        },

        getWatcherStatus () {
            let timer = setInterval(() => {
                const watcherStatus = watcher.isLive() 
                const archiveFinished = (window.archiveStatus !== 'online') && (window.archiveStatus !== undefined);
                const daemonStatus = (this.processStatus.kungfuDaemon === 'online') && this.daemonIsLive;

                this.$set(this.loadingData, 'archive', archiveFinished);
                this.$set(this.loadingData, 'watcher', watcherStatus);
                this.$set(this.loadingData, 'daemon', daemonStatus);

                const { archive, daemon } = this.loadingData;
                if (archive && this.loadingData.watcher && daemon) {
                    clearTimeout(timer)
                }

            }, 100)
        },

 

        dealGatewayStates (gatewayStates) {
            gatewayStates.forEach(gatewayState => {
                const { processId } = gatewayState;
                this.$store.dispatch('setOneMdTdState', {
                    id: processId,
                    stateData: gatewayState
                })

                //做柜台就绪回调
                this.emitMdTdStateChange(processId, gatewayState)
            });
        },  

        emitMdTdStateChange (processId, stateData) {

            this.mdTdOldState = this.mdTdOldState || {};
            
            const state = stateData.state;
            const oldState = (this.mdTdOldState[processId] || {}).state || "";

            //两种情况，一种是从等待 or others 变为就绪
            //一种是从无直接100
            if (state !== oldState || oldState === "") {
                    if (state == 100 && oldState !== 100) {
                        this.$bus.$emit('mdTdStateReady', { processId, state })
                    }
            }

            this.mdTdOldState[processId] = stateData || {};
            
        },

        removeKeyDownEvent () {
            //解除回车带来的一些不好的影响
            //比如页面重新刷新的问题
            document.body.addEventListener('keydown', (event) => {
                if(event.keyCode == 13) {
                    event.preventDefault()
                }
            })
        },      
    }
}



</script>

<style lang="scss">
@import '@/assets/scss/base.scss';

#app{
    height: 100%;
    background: $login-bg;
}



</style>