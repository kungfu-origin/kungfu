<template>
    <div id="app">
        <router-view></router-view>
        <GlobalSettingDialog
            v-if="globalSettingDialogVisiblity"
            :visible.sync="globalSettingDialogVisiblity"
        >
        </GlobalSettingDialog>

        <el-dialog
        title="系统提示"
        class="system-prepare-dialog"
        :visible="watcherLoading"
        :show-close="false"
        :close-on-click-modal="false"
        width="450px"
        >
            <div style="margin: 10px 0 20px">
                <tr-status :value="loadingData.archive ? '100' : '3'" :hasText="false"></tr-status>
                {{ loadingData.archive ? '功夫归档完成 ✓' : '功夫归档中...' }}
            </div>
            <div style="margin: 10px 0 20px">
                <tr-status :value="loadingData.watcher ? '100' : '3'" :hasText="false"></tr-status>
                {{ loadingData.watcher ? '功夫环境准备完成 ✓' : '功夫环境准备中...' }}
            </div>
            <div style="margin: 10px 0 20px">
                <tr-status :value="loadingData.daemon ? '100' : '3'" :hasText="false"></tr-status>
                {{ loadingData.daemon ? '功夫数据通信已建立 ✓' : '等待功夫数据通信建立...' }}
            </div>
        </el-dialog>

        <el-dialog
        title="系统提示"
        class="system-prepare-dialog"
        :visible.sync="recordBeforeQuitLoading"
        :show-close="false"
        :close-on-click-modal="false"
        width="450px"
        >
            <div style="margin: 10px 0 20px">
                <tr-status :value="'3'" :hasText="false"></tr-status>
                退出前保存数据中，请勿关闭
            </div>
        </el-dialog>
    </div>
</template>
<script>

import { mapState } from 'vuex';
import moment from "moment"

import GlobalSettingDialog from '@/components/Base/GlobalSettingDialog';
import { buildMarketDataPipeByDaemon, buildInstrumentsPipeByDaemon, buildTradingDataAccountPipeByDaemon, buildKungfuGlobalDataPipeByDaemon } from '@/ipcMsg/daemon';
import { watcher } from '__io/kungfu/watcher';

import ipcListenerMixin from '@/ipcMsg/ipcListenerMixin';
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';
import workersMixin from '@/workers/workersMixin';


export default {
    name: 'app',

    mixins: [ ipcListenerMixin, tickerSetMixin, workersMixin ],

    data() {
        this.kungfuGloablDataObserver = null;
        this.oldInstruments = Object.freeze(JSON.parse(localStorage.getItem('instruments') || "[]"));
        return {

            globalSettingDialogVisiblity: false,

            loadingData: {
                archive: false,
                watcher: false,
                daemon: false,
            }
        }
    },

    components: {
        GlobalSettingDialog
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
        this.bindInstrumentsDataListener();
        this.bindQuotesListener();

        this.getWatcherStatus();
    },

    computed: {
        ...mapState({
            watcherIsLive: state => state.BASE.watcherIsLive || false,
            processStatus: state => state.BASE.processStatus || {},
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
        bindInstrumentsDataListener () {
            buildInstrumentsPipeByDaemon().subscribe(data => {
                const instruments = data['instruments'] || [];

                if (!instruments || !instruments.length) {
                    localStorage.setItem('instrumentsSavedDate', '')
                    return;
                }
                
                if (this.getIfSaveInstruments(instruments || [])) {
                    localStorage.setItem('instrumentsSavedDate', moment().format('YYYY-MM-DD-HH-mm'))
                    localStorage.setItem('instruments', JSON.stringify(instruments))
                    this.oldInstruments = instruments; //refresh old instruments
                }
            })
        },

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

        getIfSaveInstruments (newInstruments) {

            if (newInstruments.length !== this.oldInstruments.length) {
                return true;
            }

            const instrumentsSavedDate = localStorage.getItem('instrumentsSavedDate')
            if (!instrumentsSavedDate) {
                return true
            } else if (instrumentsSavedDate !== moment().format('YYYY-MM-DD-HH-mm')) {
                return true 
            } else {
                return false
            }
        },

        removeLoadingMask () {
            if(document.getElementById('loading')) document.getElementById('loading').remove();
        },

        getWatcherStatus () {
            let timer = setInterval(() => {
                const watcherStatus = watcher.isLive();
                const archiveFinished = (window.archiveStatus !== 'online') && (window.archiveStatus !== undefined);
                const daemonStatus = this.processStatus.kungfuDaemon === 'online';

                this.$set(this.loadingData, 'archive', archiveFinished);
                this.$set(this.loadingData, 'watcher', watcherStatus);
                this.$set(this.loadingData, 'daemon', daemonStatus);

                const { archive, daemon } = this.loadingData;
                if (archive && this.loadingData.watcher && daemon) {
                    clearTimeout(timer)
                }

            }, 100)
        },

        bindKungfuGlobalDataListener () {
            this.kungfuGloablDataObserver = buildKungfuGlobalDataPipeByDaemon().subscribe(data => {
                
                const gatewayStates = data["gatewayStates"] || [];
                gatewayStates.forEach(gatewayState => {
                    const { processId } = gatewayState;
                    this.$store.dispatch('setOneMdTdState', {
                        id: processId,
                        stateData: gatewayState
                    })

                    //做柜台就绪回调
                    this.emitMdTdStateChange(processId, gatewayState)
                })

                const watcherIsLive = data["watcherIsLive"] || false;
                this.$store.dispatch('setWatcherIsLive', watcherIsLive)
            })
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

 .system-prepare-dialog {
    .tr-dot-content {
        margin-right: 5px;
    }
 }

</style>