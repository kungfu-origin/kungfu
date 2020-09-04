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
                <tr-status :value="loadingData.archive ? '100' : '3'" :hasText="false"></tr-status>
                {{ loadingData.archive ? '功夫归档完成 ✓' : '功夫归档中...' }}
            </div>
            <div style="margin: 10px 0 20px">
                <tr-status :value="loadingData.watcher ? '100' : '3'" :hasText="false"></tr-status>
                {{ loadingData.watcher ? '功夫环境准备完成 ✓' : '功夫环境准备中...' }}
            </div>
        </el-dialog>
    </div>
</template>
<script>
import path from 'path';

import { mapState } from 'vuex';
import { ipcRenderer } from 'electron';

import GlobalSettingDialog from '@/components/Base/GlobalSettingDialog';

import { KF_HOME } from '__gConfig/pathConfig';
import { existsSync } from '__gUtils/fileUtils';
import { deepClone, delayMiliSeconds, debounce } from '__gUtils/busiUtils';
import { buildKungfuGlobalDataPipe } from '__io/kungfu/tradingData';
import { deleteProcess } from '__gUtils/processUtils';
import { watcher } from '__io/kungfu/watcher';

export default {
    name: 'app',
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
        //ipc event
        this.bindMainProcessEvent();
        this.getWatcherStatus();

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

        subKungfuGlobalData () {
            return buildKungfuGlobalDataPipe().subscribe(data => {
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