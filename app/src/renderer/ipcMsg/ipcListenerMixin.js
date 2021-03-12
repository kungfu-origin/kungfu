
import { mapState } from 'vuex';
import { ipcRenderer, remote } from 'electron';

import { watcher, dealQuote, dealPos, dealOrder, writeKungfu } from '__io/kungfu/watcher';
import { getStrategyById, updateStrategyPath } from '__io/kungfu/strategy';
import { transformTradingItemListToData } from '__io/kungfu/watcher';
import { aliveOrderStatusList } from 'kungfu-shared/config/tradingConfig';

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
                        case 'REQ_LEDGER_DATA':
                            this.resLedgerData(parentId, pm2Id, accountId, ticker, processName)
                            break;
                        case "REQ_QUOTE_DATA":
                            this.resQuoteData(pm2Id, ticker, processName)
                            this.resInstrumentInfo(pm2Id, ticker, processName)
                            break;
                        case "REQ_POS_ORDER_DATA":
                            this.resPosData(pm2Id, accountId, processName)
                            this.resOrderData(pm2Id, parentId, processName)
                            break
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
                                this.$message.warning(`距离交易任务 ${processName} 开始执行还有 ${minute} 分钟，目前还未收到订阅行情，请检查交易进程与行情进程运行`)
                            } else {
                                this.$message.info(`距离交易任务 ${processName} 开始执行还有 ${minute} 分钟，请保证交易进程与行情进程运行`)
                            }
                            break
                        case 'CALC_AVG_VOLUME': //周均成交量
                            this.$store.dispatch('setMarketAvgVolume', packetData.body)
                            break
                        case 'REQ_HIS_AVG_VOLUME': //历史均成交量
                            const { days } = packetData.body || {};
                            this.sendResDataToProcessId("HIS_AVG_VOLUME", pm2Id, processName, { avgVolume: this.marketAvgVolume[days] || {} })
                            break
                        case 'REQ_RECORD_DATA':
                            const { mode, data } = packetData.body
                            writeKungfu(processName, mode, 'task', JSON.stringify(data))
                    }
                })
            })
        },

        resQuoteData (pm2Id, tickers, processName) {
            if (!watcher.isLive()) return;
            watcher.step();
            const ledger = watcher.ledger;

            const quotes = Object.values(ledger.Quote || {})
                .filter(quote => tickers.includes(`${quote.instrument_id}_${quote.exchange_id}`))
                .map(quote => dealQuote(quote));

            this.sendResDataToProcessId("QUOTE_DATA", pm2Id, processName, { quotes })    
        },

        resPosData (pm2Id, accountId, processName) {
            if (!watcher.isLive()) return;
            watcher.step();
            const ledger = watcher.ledger;
            const positions = Object.values(ledger.Position || {});
            const positionsResolved = transformTradingItemListToData(positions, 'account')[accountId] || [];

            this.sendResDataToProcessId("POS_DATA", pm2Id, processName, { 
                positions: positionsResolved,
            })
        },

        resOrderData (pm2Id, parentId, processName) {
            if (!watcher.isLive()) return;
            watcher.step();
            const ledger = watcher.ledger;
            const orders = this.getTargetOrdersByParentId(ledger.Order || {}, parentId)

            this.sendResDataToProcessId("ORDER_DATA", pm2Id, processName, { 
                orders
            })
        },

        resInstrumentInfo (pm2Id, tickers, processName) {
            if (!watcher.isLive()) return;
            watcher.step();
            const ledger = watcher.ledger;
            const instruments = Object.values(ledger.Instrument)
                .filter(item => tickers.includes(`${item.instrument_id}_${item.exchange_id}`))
                
            this.sendResDataToProcessId('INSTRUMENT_DATA', pm2Id, processName, {
                instruments
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

        resLedgerData (parentId, pm2Id, accountId, ticker, processName) {
            if (watcher.isLive()) {
                watcher.step();
                const ledger = watcher.ledger;
                const { orders, positions, quotes } = this.buildLedgerDataForTask(ledger, accountId, parentId, ticker)
                _pm2.sendDataToProcessId({
                    type: 'process:msg',
                    parentId,
                    data: {
                        positions: positions.map(pos => dealPos(pos)),
                        quotes: quotes.map(quote => dealQuote(quote)),
                        orders: orders.map(order => dealOrder(order))
                    },
                    id: pm2Id,
                    topic: 'LEDGER_DATA'
                }, err => {
                    if (err) {
                        console.error(processName, err)
                    }
                })
            }
        },

        //pos, quote, orders
        buildLedgerDataForTask (ledger, accountId, parentId, ticker) {
            const positions = Object.values(ledger.Position || {});
            const positionsResolved = transformTradingItemListToData(positions, 'account')[accountId] || [];
            const quotes = Object.values(ledger.Quote || {}).filter(quote => {
                return quote.instrument_id === ticker
            })
            const orders = this.getTargetOrdersByParentId(ledger.Order, parentId)
            return {
                positions: positionsResolved,
                quotes,
                orders
            }
        },

        getTargetOrdersByParentId (orders, parentId) {
            return Object.values(orders || {})
                .filter(order => {
                    if (!order.parent_id) return false
                    return (order.parent_id || '').toString() === (parentId || '').toString()
                });
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
                            .then(() => {
                                ipcRenderer.sendSync('record-before-quit-done')
                            })
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
    },
}