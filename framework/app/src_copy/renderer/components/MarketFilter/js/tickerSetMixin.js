
import { mapGetters, mapState } from 'vuex';


import { checkAllMdProcess, getIndexFromTargetTickers, findTargetFromArray, delayMilliSeconds, debounce } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { sendDataToDaemonByPm2 } from '@kungfu-trader/kungfu-uicc/utils/processUtils';
import { getTickerSets, addSetTickerSet, removeTickerSetByName } from '@kungfu-trader/kungfu-uicc/io/actions/market';
import { kungfuSubscribeInstrument } from '@kungfu-trader/kungfu-uicc/io/kungfu/makeCancelOrder';
import { encodeKungfuLocation } from '@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils';
import { watcher } from '@kungfu-trader/kungfu-uicc/io/kungfu/watcher';

export default {

    mounted () {
        this.getTickerSets();
    },

    data () {
        return {
            setTickerSetDialogInput: {},
        }
    },

    computed: {
        ...mapState({
            tickerSets: state => state.MARKET.tickerSets,
            currentTickerSetName: state => (state.MARKET.currentTickerSet || {}).name || '',
            currentTickerSetTickers: state => (state.MARKET.currentTickerSet || {}).tickers || [],
            currentTickerSet: state => state.MARKET.currentTickerSet || {},
            processStatus: state => state.BASE.processStatus || {},
        }),

        ...mapGetters([
            "flatternTickers"
        ]),

        currentTickerSetTickersResolved () {
            return this.currentTickerSetTickers
                .slice(0)
                .sort((ticker1, ticker2) => {
                    return ticker1.instrumentId.localeCompare(ticker2.instrumentId)
                })
        }
    },

    methods: {

        handleSetCurrentTickerSet(tickerSet) {
            this.$store.dispatch('setCurrentTickerSet', tickerSet)
        },

        handleRemoveTickerSet (tickerSet, replace = false) {
            const confirmPromise = replace ? Promise.resolve(true) : this.$confirm(`删除标的池 ${tickerSet.name} 会删除所有相关信息，确认删除吗？`, '提示', {confirmButtonText: '确 定', cancelButtonText: '取 消'});
            return confirmPromise
                .then(() => {
                    return removeTickerSetByName(tickerSet.name)
                })
                .then(() => {
                    if (!replace) {
                        this.$message.success('操作成功！')
                    }
                })
                .then(() => {
                    if (!replace) {
                        this.getTickerSets()
                    }
                })
                .catch(err => {
                    if (err === 'cancel') return; 
                    this.$message.error(err.message)
                })
        },

        handleConfirmAddSetTickerSet (tickerSet) {    
            return addSetTickerSet(tickerSet)
                .then(() => {
                    this.$message.success(`操作成功！`)
                    this.getTickerSets()
                })
                .catch(err => {
                    this.$message.error(err.message || `操作失败！`)
                })
        },

        handleAddTickerConfirm (tickerData, targetTickerSetName) {
            if (!targetTickerSetName) return;
            const targetTickerSet = findTargetFromArray(this.tickerSets, 'name', targetTickerSetName);
            if (!targetTickerSet) return;
            const { name, tickers } = targetTickerSet;
            const targetIndex = getIndexFromTargetTickers(tickers || {}, tickerData)
            let newTickers = tickers.slice(0);
            
            if (targetIndex === -1) {
                newTickers.push(tickerData)
            } else {
                newTickers.splice(targetIndex, 1, tickerData)
            }

            this.handleConfirmAddSetTickerSet({
                name,
                tickers: newTickers
            })
        },

        handleDeleteTicker (ticker, targetTickerSet = null) {
            if (!targetTickerSet) return;

            const { name, tickers } = targetTickerSet;
            const targetIndex = getIndexFromTargetTickers(tickers, ticker)

            if (targetIndex !== -1) {
                let targetTickers = tickers.slice(0)
                targetTickers.splice(targetIndex, 1);
                this.handleConfirmAddSetTickerSet({
                    name, 
                    tickers: targetTickers
                })
            }
        },

        bindMdTdStateChangeEvent () {
            this.$bus.$on('mdTdStateReady', ({ processId }) => {
                if (processId.includes('md')) {
                    this.subscribeTickersByProcessId(processId)
                }
            })
        },

        getTickerSets () {
            return getTickerSets()
                .then(res => {
                    this.$store.dispatch('setTickerSets', Object.freeze(res))
                    this.initUpdateCurrentTickerSet(res)
                })
        },

        initUpdateCurrentTickerSet (tickerSets) {
            if (tickerSets.length) {
                const currentTickerSetIndex = this.tickerSets.findIndex(item => item.name === this.currentTickerSetName)
                
                if (!this.currentTickerSetName) {
                    this.$store.dispatch('setCurrentTickerSet', tickerSets[0])
                } else {
                    if (currentTickerSetIndex !== -1) {
                        this.$store.dispatch('setCurrentTickerSet', tickerSets[currentTickerSetIndex])
                    } else {
                        this.$store.dispatch('setCurrentTickerSet', tickerSets[0])
                    }
                }
            } else {
                this.$store.dispatch('setCurrentTickerSet', null)
            }
        },

        //通过md 订阅
        async subscribeTickersByProcessId (mdProcessId, slience = true) {
            const sourceName = mdProcessId.split("_")[1];
            if (!sourceName) return;
            const mdLocation = encodeKungfuLocation(sourceName, 'md');
            if (!watcher.isReadyToInteract(mdLocation)) {
                await delayMilliSeconds(1000);
                await this.subscribeTickersByProcessId(mdProcessId, slience);
            } else {
                this.subscribeAllTickers(slience)                
            }
        },

        subscribeAllTickers (slience = true) {
            if (!watcher.isLive()) return;
            const tickers = this.flatternTickers || [];
            this.subscribeTickers(tickers, slience);
        },

        subscribeTickersInTickerSet (tickerSet, slience = true) {
            const target = findTargetFromArray(this.tickerSets, 'name', tickerSet)
            if (target) {
                this.subscribeTickers(target.tickers, slience)
            } else {
                this.$message.warning(`${tickerSet} 标的池不存在`)
            }
        },

        subscribeTickers (tickers, slience = true) {
            console.log("subscribeTickers", tickers.map(item => `${item.instrumentId}_${item.exchangeId}`))
            if (!watcher.isLive()) return;
            sendDataToDaemonByPm2('MAIN_RENDERER_SUBSCRIBED_TICKERS', tickers)
            tickers.forEach(ticker => {
                const { instrumentId, source, exchangeId } = ticker;
                kungfuSubscribeInstrument(source, exchangeId, instrumentId)
            })

            if (!slience) {
                if (checkAllMdProcess.call(this, tickers, this.processStatus)) {
                    this.$message.success('订阅请求已发送，请稍后')                
                }
            }
        }
    }    
}