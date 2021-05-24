
import { mapGetters, mapState } from 'vuex';

import AddSetTickerSetDialog from '@/components/MarketFilter/components/AddSetTickerSetDialog';
import AddTickerDialog from '@/components/MarketFilter/components/AddTickerDialog';

import { checkAllMdProcess, findTargetFromArray, delayMiliSeconds, debounce } from '__gUtils/busiUtils';
import { sendDataToDaemonByPm2 } from "__gUtils/processUtils";
import { getTickerSets, addSetTickerSet, removeTickerSetByName } from '__io/actions/market';
import { kungfuSubscribeTicker } from '__io/kungfu/makeCancelOrder'

export default {

    mounted () {
        this.getTickerSets();
        this.handleMdTdStateChange();
    },

    beforeDestroy() {
        this.$bus.$off('mdTdStateReady');
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
            processStatus: state => state.BASE.processStatus || {},
            watcherIsLive: state => state.BASE.watcherIsLive || false,
        }),

        ...mapGetters([
            "flatternTickers"
        ])
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

        handleMdTdStateChange () {
            const self = this;
            this.$bus.$on('mdTdStateReady', debounce(function({ processId }) {
                if (processId.includes('md')) {
                    self.subscribeTickersByProcessId(processId)
                }
            }, 2000))
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
            if (!this.watcherIsLive) return;
            const tickers = this.flatternTickers || [];
            this.subscribeTickers(tickers, slience)
            sendDataToDaemonByPm2('MAIN_RENDERER_SUBSCRIBED_TICKERS', tickers)
        },

        //通过md 订阅
        subscribeTickersByProcessId (mdProcessId, slience = true) {
            this.subscribeAllTickers(slience)
        },

        subscribeTickersInTickerSet (tickerSet, slience = true) {
            const target = findTargetFromArray(this.tickerSets, 'name', tickerSet)

            if (target) {
                this.subscribeTickers(target.tickers, slience)
            } else {
                this.$message.warning(`${tickerSet} 标的池不存在`)
            }
        },

        async subscribeTickers (tickers, slience = true) {
            if (!this.watcherIsLive) return;

            let i = 0, len = tickers.length;
            for (i; i < len; i++) {
                const ticker = tickers[i];
                const { instrumentId, source, exchangeId } = ticker;
                kungfuSubscribeTicker(source, exchangeId, instrumentId)
                await delayMiliSeconds(300)
            }

            if (!slience) {
                if (checkAllMdProcess.call(this, tickers, this.processStatus)) {
                    this.$message.success('订阅请求已发送，请稍后')                
                }
            }
        }
    }    
}