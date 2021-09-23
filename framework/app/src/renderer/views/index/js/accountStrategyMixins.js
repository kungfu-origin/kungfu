
const remote = require('electron').remote;
import { openVueWin } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { dealOrder, dealTrade } from '@kungfu-trader/kungfu-uicc/io/kungfu/watcher';

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

        isHistoryDataTrade (val) {
            if (!val) return;
            this.trades = this.getHistoryData("trade")
        },

        isHistoryDataOrder (val) {
            if (!val) return;
            this.orders = this.getHistoryData("order")
        }
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

        isHistoryDataOrder () {
            return this.isHistoryData("order")
        },

        isHistoryDataTrade () {
            return this.isHistoryData("trade")
        }
    },

    methods: {

        handleMakeOrderByPos (orderInput) {
        
            if (this.moduleType !== 'strategy') {
                const { offset, side, volume } = this.getSideOffsetVolume(orderInput)
                this.$bus.$emit('update:make-order', {
                    currentId: this.currentIdInAccountStrategyResolved || '',
                    moduleType: this.moduleType,
                    orderInput: Object.freeze({
                        ...orderInput,
                        side,
                        offset,
                        volume
                    })
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
            this.$set(this.historyData, type, { date, data })
        },

        getSideOffsetVolume (orderInput)  {
            if (!orderInput) {
                return {
                    side: 0,
                    offset: 0,
                    volume: 0
                }
            }

            const { directionOrigin, totalVolume, yesterdayVolume } = orderInput;
            let side, offset, volume;
            if (directionOrigin == 0) {
                side = 1;
            } else {
                side = 0;
            }
            
            if (+yesterdayVolume) {
                offset = 1;
                volume = +yesterdayVolume;
            } else {
                offset = 2; //closeToday;
                volume = +totalVolume || 0;
            }

            return { side, offset, volume };
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
            const { offset, side, volume } = this.getSideOffsetVolume(orderInput);
            window.makeOrderWin.webContents.send('init-make-order-win-info', {
                moduleType: this.moduleType,
                currentId: this.currentIdInAccountStrategyResolved,
                orderInput: {
                    ...orderInput,
                    offset,
                    side,
                    volume

                },
            })
        },

        bindMakeOrderWinEvent (orderInput) {
            window.makeOrderWin.once('show', () => {
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