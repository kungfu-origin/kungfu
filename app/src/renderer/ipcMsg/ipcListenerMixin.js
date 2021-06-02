
import { mapState } from 'vuex';
import { ipcRenderer, remote } from 'electron';
import moment from 'moment';
import path from 'path';

import { watcher, writeKungfuTimeValue, getTargetOrdersByParentId, flatternOrderTrades } from '__io/kungfu/watcher';
import { kungfu, getKungfuDataByDateRange } from '__io/kungfu/kungfuUtils';
import { getStrategyById, updateStrategyPath } from '__io/kungfu/strategy';
import { aliveOrderStatusList } from 'kungfu-shared/config/tradingConfig';
import { ensureLedgerData } from '__gUtils/busiUtils';
import { writeCSV } from '__gUtils/fileUtils';


import makeOrderCoreMixin from '@/components/Base/makeOrder/js/makeOrderCoreMixin';
import recordBeforeQuitMixin from "@/assets/mixins/recordBeforeQuitMixin";
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';

const { _pm2, sendDataToProcessIdByPm2 } = require('__gUtils/processUtils');
const { dialog, BrowserWindow, shell } = remote;

//一直启动，无需remove listener
export default {

    mixins: [ makeOrderCoreMixin, recordBeforeQuitMixin, tickerSetMixin ],

    data () {
        this.BUS = null;
        return {
            //导出全部历史
            exportAllTradingDataByDateDateRangeDialogVisiblity: false,
            exportAllTradingDataByDateDateRangeDialogLoading: false,
  
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
            accountsAsset: state => state.ACCOUNT.accountsAsset,
            marketAvgVolume: state => state.MARKET.marketAvgVolume || {},
        })
    },

    methods: {

        handleConfirmDateRangeForExportAllTradingData (date) {
            this.exportAllTradingDataByDateDateRangeDialogLoading = true;
            return getKungfuDataByDateRange(date)
                .then(kungfuData => {

                    if (kungfuData) {
                        this.$message.warning("导出失败，请重试！")
                    }

                    this.exportAllTradingDataByDateDateRangeDialogLoading = false;
                    this.exportAllTradingDataByDateDateRangeDialogVisiblity = false;

                    const orders = ensureLedgerData(kungfuData.Order, 'update_time'); 
                    const ordersResolved = flatternOrderTrades(orders);
                    const trades = ensureLedgerData(kungfuData.Trade, 'trade_time');
                    const tradesResolved = flatternOrderTrades(trades);
                    const orderStat = ensureLedgerData(kungfuData.OrderStat, 'insert_time');
                    const positions = ensureLedgerData(kungfuData.Position);
                    const dateResolved = moment(date).format('YYYY-MM-DD');

                    dialog.showOpenDialog({
                        properties: ['openDirectory']
                    }, targetFolder => {
                        if (!targetFolder || !targetFolder[0]) return;
                        targetFolder = targetFolder[0];
                        const ordersFilename = path.join(targetFolder, `orders-${dateResolved}`);
                        const tradesFilename = path.join(targetFolder, `trades-${dateResolved}`);
                        const orderStatFilename = path.join(targetFolder, `orderStats-${dateResolved}`);
                        const posFilename = path.join(targetFolder, `pos-${dateResolved}`);

                        writeCSV(ordersFilename, ordersResolved)
                        writeCSV(tradesFilename, tradesResolved)
                        writeCSV(orderStatFilename, orderStat)
                        writeCSV(posFilename, positions)

                        shell.showItemInFolder(targetFolder)
                        this.$message.success('导出成功！')
                    })
                })
                .finally(() => {
                    this.exportAllTradingDataByDateDateRangeDialogLoading = false;
                    this.exportAllTradingDataByDateDateRangeDialogVisiblity = false;
                })
        },

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
                            const ordersByParentId = getTargetOrdersByParentId(watcher.ledger.Order, parentId);
                            ordersByParentId
                                .filter(order => aliveOrderStatusList.includes(+(order.status || 0)))
                                .forEach(order => {
                                    this.cancelOrder('account', order)
                                })
                            break

                        case "SUBSCRIBE_BY_TICKER":
                            const sourceName = accountId ? (accountId || '').toSourceName() : sourceId;
                            this.subscribeTickers([{
                                source: sourceName,
                                exchangeId,
                                instrumentId: ticker
                            }])
                            break;

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
                            break;

                        case 'REQ_HIS_AVG_VOLUME': //历史均成交量
                            const { days } = packetData.body || {};
                            sendDataToProcessIdByPm2("HIS_AVG_VOLUME", pm2Id, processName, { avgVolume: this.marketAvgVolume[days] || {} })
                            break;

                        case 'REQ_RECORD_DATA':
                            const { data } = packetData.body;
                            const kungfuTimeNow = watcher.now();
                            const kungfuTimeNowStr = kungfu.formatTime(kungfuTimeNow, '%H:%M:%S.%N').slice(0, 12);

                            writeKungfuTimeValue(processName, '', 'task', JSON.stringify({
                                ...data,
                                kungfuTimeNow: kungfuTimeNow.toString(),
                                kungfuTimeNowStr
                            }))
                    }
                })
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
                        localStorage.setItem("clearJournalDate", "")
                        this.$message.success('清理 journal 完成，请重启应用！')
                        break
                    case "open-log":
                        dialog.showOpenDialog({
                            properties: ['openFile']
                        }, (logPath) => {
                            if(!logPath || !logPath[0]) return;
                            this.$showLog(logPath[0])
                        })
                        break;
                    case "export-all-trading-data":
                        //App.vue
                        this.exportAllTradingDataByDateDateRangeDialogVisiblity = true;

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
        }        
    },
}