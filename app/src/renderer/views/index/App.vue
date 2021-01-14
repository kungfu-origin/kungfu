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
        :visible.sync="watcherLoading"
        :show-close="false"
        :close-on-click-modal="false"
        width="450px"
        >
            <div style="margin: 10px 0 20px">
                <tr-status :value="loadingData.watcher ? '100' : '3'" :hasText="false"></tr-status>
                {{ loadingData.watcher ? '功夫环境准备完成 ✓' : '功夫环境准备中...' }}
            </div>
            <div style="margin: 10px 0 20px">
                <tr-status :value="loadingData.archive ? '100' : '3'" :hasText="false"></tr-status>
                {{ loadingData.archive ? '功夫归档完成 ✓' : '功夫归档中...' }}
            </div>
        </el-dialog>
    </div>
</template>
<script>
import path from 'path';

import { mapState } from 'vuex';

import GlobalSettingDialog from '@/components/Base/GlobalSettingDialog';

import { existsSync } from '__gUtils/fileUtils';
import { deepClone, delayMiliSeconds, debounce } from '__gUtils/busiUtils';
import { buildKungfuGlobalDataPipe, buildTradingDataPipe } from '__io/kungfu/tradingData';
import { watcher } from '__io/kungfu/watcher';

import ipcListenerMixin from '@/ipcMsg/ipcListenerMixin.js';

export default {
    name: 'app',

    mixins: [ ipcListenerMixin ],

    data() {
        this.kungfuGloablDataObserver = null;
        return {
            watcherLoading: false,
            globalSettingDialogVisiblity: false,

            loadingData: {
                archive: false,
                watcher: false
            }
        }
    },

    components: {
        GlobalSettingDialog
    },

    mounted(){

        this.removeLoadingMask();
        this.removeKeyDownEvent();

        this.$store.dispatch('getTdMdList');
        this.$store.dispatch('getStrategyList');
        this.$store.dispatch('getAccountSourceConfig');
        this.$store.dispatch('getKungfuConfig');

        this.bindKungfuGlobalDataListener();
        this.bindTradingDataListener();

        this.getWatcherStatus();

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
            if(document.getElementById('loading')) document.getElementById('loading').remove();
        },

        getWatcherStatus () {
            let timer = setInterval(() => {
                const watcherStatus = watcher.isLive();
                const archiveFinished = (window.archiveStatus !== 'online') && (window.archiveStatus !== undefined);

                this.$set(this.loadingData, 'archive', archiveFinished);
                this.$set(this.loadingData, 'watcher', watcherStatus);

                if (!archiveFinished) this.watcherLoading = true;
                else if (!watcherStatus) this.watcherLoading = true;
                else {
                    this.watcherLoading = false;
                    clearInterval(timer)
                }

            }, 500)
        },

        bindKungfuGlobalDataListener () {
            this.kungfuGloablDataObserver = buildKungfuGlobalDataPipe().subscribe(data => {
                data.gatewayStates.forEach(gatewayState => {
                    const { processId } = gatewayState;
                    this.$store.dispatch('setOneMdTdState', {
                        id: processId,
                        stateData: gatewayState
                    })
                })
            })
        },

        bindTradingDataListener () {
            this.tradingDataPipe = buildTradingDataPipe('account').subscribe(data => {
                const assets = data['assets'];
                this.$store.dispatch('setAccountsAsset', Object.freeze(JSON.parse(JSON.stringify(assets))));
            })
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