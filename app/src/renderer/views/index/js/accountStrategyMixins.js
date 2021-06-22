
const remote = require('electron').remote;
import { openVueWin } from '__gUtils/busiUtils';
import { dealOrder, dealTrade } from '__io/kungfu/watcher';

export default {

    data () {

        return {
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

        handleMakeOrderByPos (orderInput) {
            orderInput = Object.freeze(orderInput)
            if (this.moduleType !== 'strategy') {
                this.$bus.$emit('update:make-order', {
                    currentId: this.currentIdInAccountStrategyResolved || '',
                    moduleType: this.moduleType,
                    orderInput
                })
            } else {
                this.handleShowOrCloseMakeOrderDashboard(orderInput)
            }

        },

        handleShowOrCloseMakeOrderDashboard (orderInput) {
            return this.buildMakeOrderWin(orderInput)
                .then(() => {
                    this.emitCurrentMakeOrderWinInfo(orderInput);
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

        handleMakeOrderByOrderBook (data) {
            console.log(data)
        },

        isMakeOrderWinIsDestroyed () {
            if (!window.makeOrderWin) return true;
            if (window.makeOrderWin.isDestroyed()) return true;
            return false;
        },

        buildMakeOrderWin (orderInput) {
            //仅在strategy时创建窗口
            if ((!window.makeOrderWin || this.isMakeOrderWinIsDestroyed()) && this.moduleType === 'strategy') {
                return openVueWin(
                    'makeOrder', 
                    `/make-order`, 
                    remote, 
                    { width: 470, height: 520 }
                ).then((win) => {
                    window.makeOrderWin = win;
                    window.makeOrderWin.setAlwaysOnTop(true);
                    this.bindMakeOrderWinEvent(orderInput);
                })
            } else {
                return Promise.resolve(true)
            }
        },

        emitCurrentMakeOrderWinInfo (orderInput) {
            if (!window.makeOrderWin || this.isMakeOrderWinIsDestroyed()) return;

            window.makeOrderWin.webContents.send('init-make-order-win-info', {
                moduleType: this.moduleType,
                currentId: this.currentIdInAccountStrategyResolved,
                orderInput,
            })
        },

        bindMakeOrderWinEvent (orderInput) {
            window.makeOrderWin.on('show', () => {
                this.emitCurrentMakeOrderWinInfo(orderInput);
            })
        },

        isHistoryData (type) {
            if (type === 'order') {
                return !!(this.historyData['order'] && ((this.historyData['order'] || {}).date))
            } else if (type === 'trade') {
                return !!(this.historyData['trade'] && ((this.historyData['trade'] || {}).date))
            } else {
                console.error('isHistoryData type is not trade or order!')
                return false;
            }
        },

        getHistoryData (type) {
            if (type === 'order') {
                return Object.freeze((this.historyData['order'].data || []).map(item => {
                    return  Object.freeze({
                        ...item.orderStats,
                        ...dealOrder(item),
                    })
                }));
            } else if (type === 'trade') {
                return this.trades = Object.freeze((this.historyData['trade'].data || []).map(item => {
                    return Object.freeze({
                        ...item.orderStats,
                        ...dealTrade(item),
                    })
                }));
            } else {
                console.error('getHistoryData type is not trade or order!')
                return []
            }
        }        
    }
}