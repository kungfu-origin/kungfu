
import { mapState } from 'vuex';
import { ipcRenderer, remote } from 'electron';
import { buildTradingDataPipe } from '__io/kungfu/tradingData';
import makeOrderCoreMixin from '@/components/Base/tradingData/js/makeOrderCoreMixin';

const BrowserWindow = remote.BrowserWindow;

//一直启动，无需remove listener
export default {

    mixins: [ makeOrderCoreMixin ],

    mounted () {
        this.bindIPCListener();
        this.bindTradingDataListener();
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

        bindTradingDataListener () {
            this.tradingDataPipe = buildTradingDataPipe('account').subscribe(data => {
                const assets = data['assets'];
                this.$store.dispatch('setAccountsAsset', Object.freeze(JSON.parse(JSON.stringify(assets))));
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

            ipcRenderer.on('ipc-emit-accountSourceConfig', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                return this.$store.dispatch('getAccountSourceConfig')
                    .then(accountSource => {
                        childWin.webContents.send('ipc-res-accountSourceConfig', Object.freeze(accountSource))
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
        },
    },
}