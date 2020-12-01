
import { mapState } from 'vuex';
import { ipcRenderer, remote } from 'electron';

import { getStrategyById, updateStrategyPath } from '__io/kungfu/strategy';

import makeOrderCoreMixin from '@/components/Base/makeOrder/js/makeOrderCoreMixin';

const BrowserWindow = remote.BrowserWindow;

//一直启动，无需remove listener
export default {

    mixins: [ makeOrderCoreMixin ],

    data () {
        return {
            globalSettingDialogVisiblity: false
        }
    },

    mounted () {
        this.bindIPCListener();
        this.bindMainIPCListener();
    },

    destroyed(){
        this.tradingDataPipe && this.tradingDataPipe.unsubscribe();
    },

    computed: {
        ...mapState({
            accountsAsset: state => state.ACCOUNT.accountsAsset,
        })
    },

    methods: {

        bindMainIPCListener () {
            ipcRenderer.on('main-process-messages', (event, args) => {
                switch (args) {
                    case 'open-setting-dialog':
                        this.globalSettingDialogVisiblity = true;
                        break
                }
            })
        },
        
        bindIPCListener () {

            ipcRenderer.on('ipc-emit-tdMdList', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                return this.$store.dispatch('getTdMdList')
                    .then(({ tdList, mdList }) => {
                        childWin.webContents.send('ipc-res-tdMdList', {
                            tdList: Object.freeze(tdList),
                            mdList: Object.freeze(mdList)
                        })
                    })
            })

            ipcRenderer.on('ipc-emit-strategyList', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                return this.$store.dispatch('getStrategyList')
                    .then(strategyList => {
                        childWin.webContents.send('ipc-res-strategyList', Object.freeze(strategyList))
                    })
            })

            ipcRenderer.on('ipc-emit-accountsAsset', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                childWin.webContents.send('ipc-res-accountsAsset', Object.freeze(this.accountsAsset))
            })

            ipcRenderer.on('ipc-emit-cancelOrder', (event, { childWinId, params }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                const { moduleType, orderData, strategyId } = params;
                return this.cancelOrder(moduleType, orderData, strategyId)
                    .then(() => this.$message.success('撤单指令已发送！'))
                    .catch(err => this.$message.error(err.message || '撤单指令发送失败！'))
                    .finally(() => {
                        childWin.webContents.send('ipc-res-cancelOrder')
                    })
            })

            ipcRenderer.on('ipc-emit-makeOrder', (event, { childWinId, params }) => {

                const childWin = BrowserWindow.fromId(childWinId);
                const { moduleType, makeOrderForm, currentAccountResolved, strategyId } = params;
                return this.makeOrder(moduleType, makeOrderForm, currentAccountResolved, strategyId)
                    .then(() => this.$message.success('下单指令已发送！'))
                    .catch(err => this.$message.error(err.message || '下单指令发送失败！'))
                    .finally(() => {
                        childWin.webContents.send('ipc-res-makeOrder')
                    })
            })

            ipcRenderer.on('ipc-emit-kungfuConfig', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                return this.$store.dispatch('getKungfuConfig')
                    .then(kfConfig => {
                        childWin.webContents.send('ipc-res-kungfuConfig', Object.freeze(kfConfig))
                    })
            })

            ipcRenderer.on('ipc-emit-strategyById', (event, { childWinId, params }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                const { strategyId } = params;
                return getStrategyById(strategyId)
                    .then(strategies => {
                        childWin.webContents.send('ipc-res-strategyById', Object.freeze(strategies))
                    })
            })

            ipcRenderer.on('ipc-emit-updateStrategyPath', (event, { childWinId, params }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                const { strategyId, strategyPath } = params;
                return updateStrategyPath(strategyId, strategyPath)
                    .then(() => {
                        childWin.webContents.send('ipc-res-updateStrategyPath')
                    })
            })
        },
    },
}