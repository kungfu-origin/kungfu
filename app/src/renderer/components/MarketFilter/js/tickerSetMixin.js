
import { mapGetters, mapState } from 'vuex';

import AddSetTickerSetDialog from '@/components/MarketFilter/components/AddSetTickerSetDialog';
import AddTickerDialog from '@/components/MarketFilter/components/AddTickerDialog';

import { ifProcessRunning, findTargetFromArray } from '__gUtils/busiUtils';
import { getTickerSets, addSetTickerSet, removeTickerSetByName } from '__io/actions/market';
import { kungfuSubscribeTicker } from '__io/kungfu/makeCancelOrder'
import { watcher } from '__io/kungfu/watcher';

export default {

    mounted () {
        this.getTickerSets();
    },

    components: {
        AddSetTickerSetDialog,
        AddTickerDialog
    },

    data () {
        return {
            addSetTickerSetDialogMethod: '',
            addSetTickerSetDialogVisiblity: false,
            setTickerSetDialogInput: {},

            addTickerDialogVisiblity: false,
        }
    },

    computed: {
        ...mapState({
            tickerSets: state => state.MARKET.tickerSets,
            currentTickerSetName: state => (state.MARKET.currentTickerSet || {}).name || '',
            currentTickerSetTickers: state => (state.MARKET.currentTickerSet || {}).tickers || [],
            currentTickerSet: state => state.MARKET.currentTickerSet || {},
            processStatus: state => state.BASE.processStatus || {}
        }),

        ...mapGetters([
            "flatternTickers"
        ])
    },

    watch: {
        flatternTickers () {
            this.subscribeAllTickers()
        }
    },

    methods: {

        handleSetTickerSet (tickerSet) {
            this.addSetTickerSetDialogMethod = 'set';
            this.setTickerSetDialogInput = tickerSet;
            this.addSetTickerSetDialogVisiblity = true;
        },

        handleSetCurrentTickerSet(tickerSet) {
            this.$store.dispatch('setCurrentTickerSet', tickerSet)
        },

        handleAddTicker () {
            this.addTickerDialogVisiblity = true;
        },

        handleAddTickerSet () {
            this.addSetTickerSetDialogMethod = 'add';
            this.addSetTickerSetDialogVisiblity = true;
        },

        handleRemoveTickerSet (tickerSet) {
            this.$confirm(`删除标的池 ${tickerSet.name} 会删除所有相关信息，确认删除吗？`, '提示', {confirmButtonText: '确 定', cancelButtonText: '取 消'})
                .then(() => {
                    return removeTickerSetByName(tickerSet.name)
                })
                .then(() => {
                    this.$message.success('操作成功！')
                })
                .then(() => {
                    this.getTickerSets()
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

        handleAddTickerConfirm (tickerData) {
            this.$bus.$emit('add-ticker-for-ticker-set', Object.freeze(tickerData))
        },

        getTickerSets () {
            return getTickerSets()
                .then(res => {
                    this.$store.dispatch('setTickerSets', res)
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
                    }
                }
            } else {
                this.$store.dispatch('setCurrentTickerSet', null)
            }
        },

        subscribeAllTickers (slience = true) {
            if (!watcher.isLive()) return;
            const tickers = this.flatternTickers || [];
            this.subscribeTickers(tickers, slience)
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
            tickers.forEach(ticker => {
                const { instrumentId, source, exchangeId } = ticker;
                kungfuSubscribeTicker(source, exchangeId, instrumentId)
            })

            if (!slience) {
                const subscribeSuccess = this.checkAllMdProcess(tickers);

                if (subscribeSuccess) {
                    this.$message.success('正在订阅标的池内标的，请稍后')                
                }
            }
        },

        checkAllMdProcess (tickers) {
            let mds = {};
            tickers.forEach(item => {
                mds[item.source] = true;
            })

            const unrunningSources = Object.keys(mds || {}).filter(source => {
                const processId = `md_${source}`;
                if (!ifProcessRunning(processId, this.processStatus)) {
                    return true
                } else {
                    return false
                }
            })

            if (unrunningSources.length) {
                this.$message.warning(`${unrunningSources.join(', ')} 行情进行未开启!`)
                return true
            } else {
                return false
            }
        }
    }    
}