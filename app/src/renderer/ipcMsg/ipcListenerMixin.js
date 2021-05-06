
import { mapState } from 'vuex';
import { ipcRenderer, remote } from 'electron';
import fse from 'fs-extra';
import path from 'path';

import { watcher, dealQuote, dealPos, dealOrder, writeKungfu } from '__io/kungfu/watcher';
import { getStrategyById, updateStrategyPath } from '__io/kungfu/strategy';
import { transformTradingItemListToData } from '__io/kungfu/watcher';
import { aliveOrderStatusList } from 'kungfu-shared/config/tradingConfig';
import { KF_HOME } from '__gConfig/pathConfig';
import { listDir } from '__gUtils/fileUtils';

import makeOrderCoreMixin from '@/components/Base/makeOrder/js/makeOrderCoreMixin';
import recordBeforeQuitMixin from "@/assets/mixins/recordBeforeQuitMixin";
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';

const { _pm2 } = require('__gUtils/processUtils');

const BrowserWindow = remote.BrowserWindow;

//一直启动，无需remove listener
export default {

    mixins: [ makeOrderCoreMixin, recordBeforeQuitMixin, tickerSetMixin ],

    data () {
        this.BUS = null;
        return {
            globalSettingDialogVisiblity: false
        }
    },

    mounted () {
        this.bindIPCListener();
        this.bindPMPCListener();
    },

    destroyed(){
        this.tradingDataPipe && this.tradingDataPipe.unsubscribe();
    },

    computed: {
        ...mapState({
            watcherIsLive: state => state.BASE.watcherIsLive,
            accountsAsset: state => state.ACCOUNT.accountsAsset,
            marketAvgVolume: state => state.MARKET.marketAvgVolume || {},
        })
    },

    methods: {

        bindPMPCListener () {
            this.BUS && this.BUS.off();
            _pm2.launchBus((err, pm2_bus) => {
                this.BUS = pm2_bus
                pm2_bus.on('process:msg', (packet) => {
                    const packetData = packet.data || {};
                    const processData = packet.process || {};
                    const pm2Id = processData.pm_id;
                    const processName = processData.name;
                    const dataType = packetData.type;
                    const { 
                        accountId, 
                        exchangeId, 
                        ticker, 
                        parentId, 
                        sourceId,  
                    } = packetData.body || {};

                    switch (dataType) {
                        case 'MAKE_ORDER_BY_PARENT_ID':
                            const makeOrderData = packetData.body;
                            const markOrderDataResolved = {
                                ...makeOrderData,
                                parent_id: BigInt(makeOrderData.parent_id)
                            }
                            return this.makeOrder('account', markOrderDataResolved, makeOrderData.name)
                                .catch(err => {
                                    this.$message.error(err.message)
                                })
                        case 'CANCEL_ORDER_BY_PARENT_ID':
                            const ordersByParentId = this.getTargetOrdersByParentId(watcher.ledger.Order, parentId)
                            ordersByParentId
                                .filter(order => aliveOrderStatusList.includes(+(order.status || 0)))
                                .forEach(order => {
                                    const orderData = dealOrder(order)
                                    this.cancelOrder('account', orderData)
                                })
                            break
                        case "SUBSCRIBE_BY_TICKER":
                            
                            const sourceName = accountId ? (accountId || '').toSourceName() : sourceId;
                            this.subscribeTickers([{
                                source: sourceName,
                                exchangeId,
                                instrumentId: ticker
                            }])
                            break
                        case "SUBSCRIBE_BY_TICKERSET":
                            const { tickerSet } = packetData.body || {}
                            this.subscribeTickersInTickerSet(tickerSet)
                            break;
                        case "TIME_ALERT":
                            const { minute, quoteAlive } = packetData.body || {};
                            
                            if (!quoteAlive) {
                                this.$message.warning(`距离算法任务 ${processName} 开始执行还有 ${minute} 分钟，目前还未收到订阅行情，请检查交易进程与行情进程运行`)
                            } else {
                                this.$message.info(`距离算法任务 ${processName} 开始执行还有 ${minute} 分钟，请保证交易进程与行情进程运行`)
                            }
                            break
                        case 'REQ_HIS_AVG_VOLUME': //历史均成交量
                            const { days } = packetData.body || {};
                            this.sendResDataToProcessId("HIS_AVG_VOLUME", pm2Id, processName, { avgVolume: this.marketAvgVolume[days] || {} })
                            break
                        case 'REQ_RECORD_DATA':
                            const { data } = packetData.body;
                            writeKungfu(processName, '', 'task', JSON.stringify(data))
                    }
                })
            })
        },

        sendResDataToProcessId (topic, pm2Id, processName, data) {
            _pm2.sendDataToProcessId({
                type: 'process:msg',
                data,
                id: pm2Id,
                topic: topic
            }, err => {
                if (err) {
                    console.error(processName, err)
                }
            })
        },

        
        bindIPCListener () {
            ipcRenderer.removeAllListeners('main-process-messages')
            ipcRenderer.on('main-process-messages', (event, args) => {
                switch (args) {
                    case 'open-setting-dialog':
                        this.globalSettingDialogVisiblity = true;
                        break
                    case 'record-before-quit':
                        this.recordBeforeQuit()
                            .finally(() => {
                                ipcRenderer.sendSync('record-before-quit-done')
                            })
                        break
                    case 'clear-journal':
                        this.removeJournal(KF_HOME)
                        break

                }
            })

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

        removeJournal (targetFolder) {
            
            async function iterator (folder) {
                const items = await listDir(folder)
                
                const folders = items.filter(f => {
                    const stat = fse.statSync(path.join(folder, f))

                    if (stat.isDirectory()) return true;
                    return false;
                })

                const files = items.filter(f => {
                    const stat = fse.statSync(path.join(folder, f))

                    if (stat.isFile()) return true;
                    return false;
                })                

                files.forEach(f => {
                    if (f.includes('.journal')) {
                        fse.removeSync(path.join(folder, f))
                    }
                })

                folders.forEach(f => {
                    iterator(path.join(folder, f))
                })                
            }  

            iterator(targetFolder)
        }

        
    },
}