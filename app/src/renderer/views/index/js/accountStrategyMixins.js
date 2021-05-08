
const remote = require('electron').remote;
import { openVueWin } from '__gUtils/busiUtils';
import { dealOrder, dealTrade } from '__io/kungfu/watcher';

export default {

    data () {

        return {
            makeOrderByPosData: {},
            historyData: {},
        }
    },

    mounted () {
        this.emitCurrentMakeOrderWinInfo();
    },

    watch: {
        currentIdInAccountStrategyResolved () {
            this.emitCurrentMakeOrderWinInfo();
        },  
    },

    computed: {
        currentIdInAccountStrategyResolved () {
            if (this.moduleType === 'strategy') {
                return this.strategyId
            } else if (this.moduleType === 'account') {
                return this.currentId
            } else if (this.moduleType === 'ticker'){
                return this.currentId
            } else {
                return ''
            }
        },
    },

    methods: {

        handleMakeOrderByPos (item) {
            this.makeOrderByPosData = Object.freeze(item);
            this.$nextTick().then(() => this.handleShowOrCloseMakeOrderDashboard());
        },

        handleShowOrCloseMakeOrderDashboard () {
            return this.buildMakeOrderWin()
                .then(() => {
                    this.emitCurrentMakeOrderWinInfo();
                    window.makeOrderWin && window.makeOrderWin.show && window.makeOrderWin.show();
                    window.makeOrderWin && window.makeOrderWin.focus && window.makeOrderWin.focus();
                })        
        },
        
        handleShowHistory ({ date, data, type }) {
            this.$set(this.historyData, type, {
                date,
                data
            })
        },

        buildMakeOrderWin () {
            //仅在strategy时创建窗口
            if (!window.makeOrderWin && this.moduleType === 'strategy') {
                return openVueWin(
                    'makeOrder', 
                    `/make-order`, 
                    remote, 
                    { width: 470, height: 520 }
                ).then((win) => {
                    window.makeOrderWin = win;
                    window.makeOrderWin.setAlwaysOnTop(true);
                    this.bindMakeOrderWinEvent();
                })
            } else {
                return Promise.resolve(true)
            }
        },

        emitCurrentMakeOrderWinInfo () {
            if (!window.makeOrderWin) return;
            
            window.makeOrderWin.webContents.send('init-make-order-win-info', {
                moduleType: this.moduleType,
                currentId: this.currentIdInAccountStrategyResolved,
                makeOrderByPosData: this.makeOrderByPosData,
                currentTicker: this.currentTicker
            })
        },

        bindMakeOrderWinEvent () {

            window.makeOrderWin.on('show', () => {
                this.emitCurrentMakeOrderWinInfo();
            })
            
            window.makeOrderWin.on('close', () => {
                window.makeOrderWin = null;
                this.makeOrderByPosData = Object.freeze({})
                this.makeOrderByQuote = Object.freeze({})
            })
        },

        isHistoryData (type) {
            if (type === 'order') {
                return this.historyData['order'] && ((this.historyData['order'] || {}).date)
            } else if (type === 'trade') {
                return this.historyData['trade'] && ((this.historyData['trade'] || {}).date)
            } else {
                throw new Error('isHistoryData type is not trade or order!')
                return false;
            }
        },

        getHistoryData (type) {
            if (type === 'order') {
                return Object.freeze((this.historyData['order'].data || []).map(item => Object.freeze(dealOrder(item))));
            } else if (type === 'trade') {
                return this.trades = Object.freeze((this.historyData['trade'].data || []).map(item => Object.freeze(dealTrade(item))));
            } else {
                throw new Error('getHistoryData type is not trade or order!')
                return []
            }
        }        
    }
}