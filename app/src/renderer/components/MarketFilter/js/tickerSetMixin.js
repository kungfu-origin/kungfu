
import { mapState } from 'vuex';

import AddSetTickerSetDialog from '@/components/MarketFilter/components/AddSetTickerSetDialog';
import AddTickerDialog from '@/components/MarketFilter/components/AddTickerDialog';

import { getTickerSets, addSetTickerSet, removeTickerSetByName } from '__io/actions/market';
import { kungfuSubscribeTicker } from '__io/kungfu/makeCancelOrder'
import { watcher } from '__io/kungfu/watcher';

export default {

    mounted () {
        this.getTickerSet();
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
            currentTickerSet: state => state.MARKET.currentTickerSet || {}
        }),
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
                    this.getTickerSet()
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
                    this.getTickerSet()
                })
                .catch(err => {
                    this.$message.error(err.message || `操作失败！`)
                })
        },

        handleAddTickerConfirm (tickerData) {
            this.$bus.$emit('add-ticker-for-ticker-set', Object.freeze(tickerData))
        },

        getTickerSet () {
            return getTickerSets()
                .then(res => {
                    this.$store.dispatch('setTickerSets', res)
                    this.initUpdateCurrentTickerSet(res)
                    this.subscribeAllTickers(res)
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

        flatternTickers (tickerSets) {
            let tickersList = [];
            tickerSets.forEach(tickerSet => {
                const tickers = tickerSet.tickers || [];
                tickersList = [ ...tickersList, ...tickers]
            });

            return tickersList
        },

        subscribeAllTickers (tickerSets) {
            if (!watcher.isLive()) return;
            const tickers = this.flatternTickers(tickerSets);
            tickers.forEach(ticker => {
                const { instrumentId, source, exchangeId } = ticker;
                kungfuSubscribeTicker(source, exchangeId, instrumentId)
            })
        }
    }
    
}