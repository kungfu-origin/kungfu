
import { mapState } from 'vuex';
import { ipcRenderer, remote } from 'electron';

import { watcher, dealQuote, dealPos, dealOrder } from '__io/kungfu/watcher';
import { getStrategyById, updateStrategyPath } from '__io/kungfu/strategy';

import makeOrderCoreMixin from '@/components/Base/makeOrder/js/makeOrderCoreMixin';

const { _pm2 } = require('__gUtils/processUtils');

const BrowserWindow = remote.BrowserWindow;

//一直启动，无需remove listener
export default {

    mixins: [ makeOrderCoreMixin ],

    data () {
        this.BUS = null;
        return {
            globalSettingDialogVisiblity: false
        }
    },

    mounted () {
        this.bindIPCListener();
        this.bindMainIPCListener();
        this.bindPMPCListener();
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

        bindPMPCListener () {
            this.BUS && this.BUS.off();
            _pm2.launchBus((err, pm2_bus) => {
                this.BUS = pm2_bus
                pm2_bus.on('process:msg', (packet) => {
                    const data = packet.data;
                    const processData = packet.process;
                    const pm2Id = processData.pm_id;
                    const processName = processData.name;
                    const dataType = data.type;

                    switch (dataType) {
                        case 'REQ_LEDGER_DATA':
                            const parentId = data.parentId
                            this.resLedgerData(parentId, pm2Id, processName)
                            break;
                        case 'MAKE_ORDER_BY_PARENT_ID':
                            const makeOrderData = data.body;
                            console.log(makeOrderData)
                            return this.makeOrder('account', makeOrderData, makeOrderData.name)
                        case 'CANCEL_ORDER_BY_CLINET_ID':
                            break
                    }
                })
            })
        },

        resLedgerData (parentId, pm2Id, processName) {
            if (watcher.isLive()) {
                watcher.step();
                const ledger = watcher.ledger;
                const { orders, positions, quotes } = this.buildLedgerDataForTask(ledger, parentId)
                _pm2.sendDataToProcessId({
                    type: 'process:msg',
                    data: {
                        positions: positions.map(pos => dealPos(pos)),
                        quotes: quotes.map(quote => dealQuote(quote)),
                        orders: orders.map(orders => dealOrder(orders))
                    },
                    id: pm2Id,
                    topic: 'LEDGER_DATA'
                }, (err) => {
                    if (err) {
                        console.error(processName, err)
                    }
                })
            }
        },

        //pos, quote, orders
        buildLedgerDataForTask (ledger, parentId) {
            const positions = Object.values(ledger.Position || {});
            const quotes = Object.values(ledger.Quote || {});
            const orders = Object.values(ledger.Orders || {}).filter(order => order.parent_id === parentId);
            console.log(parentId, orders)
            return {
                positions,
                quotes,
                orders
            }
        },
        
        bindIPCListener () {

            ipcRenderer.removeAllListeners('ipc-emit-tdMdList')
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

            ipcRenderer.removeAllListeners('ipc-emit-strategyList')
            ipcRenderer.on('ipc-emit-strategyList', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                return this.$store.dispatch('getStrategyList')
                    .then(strategyList => {
                        childWin.webContents.send('ipc-res-strategyList', Object.freeze(strategyList))
                    })
            })

            ipcRenderer.removeAllListeners('ipc-emit-accountsAsset')
            ipcRenderer.on('ipc-emit-accountsAsset', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                childWin.webContents.send('ipc-res-accountsAsset', Object.freeze(this.accountsAsset))
            })

            ipcRenderer.removeAllListeners('ipc-emit-cancelOrder')
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

            ipcRenderer.removeAllListeners('ipc-emit-makeOrder')
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

            ipcRenderer.removeAllListeners('ipc-emit-kungfuConfig')
            ipcRenderer.on('ipc-emit-kungfuConfig', (event, { childWinId }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                return this.$store.dispatch('getKungfuConfig')
                    .then(kfConfig => {
                        childWin.webContents.send('ipc-res-kungfuConfig', Object.freeze(kfConfig))
                    })
            })

            ipcRenderer.removeAllListeners('ipc-emit-strategyById')
            ipcRenderer.on('ipc-emit-strategyById', (event, { childWinId, params }) => {
                const childWin = BrowserWindow.fromId(childWinId);
                const { strategyId } = params;
                return getStrategyById(strategyId)
                    .then(strategies => {
                        childWin.webContents.send('ipc-res-strategyById', Object.freeze(strategies))
                    })
            })

            ipcRenderer.removeAllListeners('ipc-emit-updateStrategyPath')
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