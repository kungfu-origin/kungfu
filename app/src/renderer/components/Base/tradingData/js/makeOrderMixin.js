
// import { kungfuCancelOrder, kungfuMakeOrder } from '__io/kungfu/makeCancelOrder';
// import { decodeKungfuLocation } from '__io/kungfu/watcher';
import { delayMiliSeconds } from '__gUtils/busiUtils';


const kungfuCancelOrder = (...params) => {
    console.log(params, 'kungfuCancelOrder')
}

const kungfuMakeOrder = (...params) => {
    console.log(params, 'kungfuMakeOrder')
}

const decodeKungfuLocation = (...params) => {
    console.log(params, 'decodeKungfuLocation')
}

export default {

    data () {

        return {
            //adjustOrder
            adjustOrderInputVisibility: false, 
            adjustOrderInputSizeData: Object.freeze({}),
            adjustOrderTargetData: {},
            adjustOrderForm: {
                name: '', // account_id in strategy
                instrument_id: '',
                instrument_type: '',
                exchange_id: '',
                limit_price: 0,
                volume: 0,
                side: 0,
                offset: 0,
                price_type: 0,
                hedge_flag: 0,
            },
            adjustOrderProp: '',
        }
    },


    methods: {

        handleShowAdjustOrder (event, row, cell) {

            if (![1, 2, 7].includes(+row.status)) {
                return
            }

            if ((cell.prop === "volumeTraded") || (cell.prop === "limitPrice")) {
                event.stopPropagation();
            } else {
                return
            }

            const targetRectData = event.target.getBoundingClientRect();
            const left = targetRectData.left + 'px';
            const top = targetRectData.top + 'px';
            const width = targetRectData.width  + 'px';
            const height = targetRectData.height  + 'px';
            
            this.adjustOrderInputSizeData = Object.freeze({
                left,
                top,
                width,
                height
            })

            this.adjustOrderProp = cell.prop;
            this.adjustOrderTargetData = Object.freeze(row);

            //build form
            this.$set(this.adjustOrderForm, 'name', row.accountId.trim())
            this.$set(this.adjustOrderForm, 'instrument_id', row.instrumentId)
            this.$set(this.adjustOrderForm, 'instrument_type', row.instrumentTypeOrigin)
            this.$set(this.adjustOrderForm, 'exchange_id', row.exchangeId)
            this.$set(this.adjustOrderForm, 'limit_price', +row.limitPrice)
            this.$set(this.adjustOrderForm, 'limitPriceOld', +row.limitPrice)
            this.$set(this.adjustOrderForm, 'volume', +row.volumeLeft)
            this.$set(this.adjustOrderForm, 'volumeOld', +row.volumeLeft)
            this.$set(this.adjustOrderForm, 'side', row.sideOrigin)
            this.$set(this.adjustOrderForm, 'offset', row.offsetOrigin)
            this.$set(this.adjustOrderForm, 'price_type', row.priceTypeOrigin)
            this.$set(this.adjustOrderForm, 'hedge_flag', row.hedgeFlagOrigin)

            this.$nextTick()
                .then(() => {
                    this.adjustOrderInputVisibility = true;
                })
        },

        handleBlurAdjustOrderInput (type) {
            const { limit_price, limitPriceOld, volume, volumeOld } = this.adjustOrderForm;

            if (!+limit_price) {
                this.clearAdjustOrderData()
                return
            }

            if (!+volume) {
                this.clearAdjustOrderData()
                return
            }

            if (type === 'price') {
                if (limit_price === limitPriceOld) {
                    return
                }
            }

            if (type === 'volume') {
                if (volume === volumeOld) {
                    return
                }
            }

            this.adjustOrderInputVisibility = false;

            this.cancelOrder(this.adjustOrderTargetData)
                .then(() => this.$message.success('撤单指令已发送！'))
                .then(() => delayMiliSeconds(1000))
                .then(() => this.makeOrder(
                    this.moduleType, 
                    this.adjustOrderForm, 
                    this.getAdjustOrderAccountResolved(), 
                    this.currentId
                ))
                .then(() => this.$message.success('下单指令已发送！'))
                .catch((err) => {
                    if(err == 'cancel') return;
                    this.$message.error(err.message || '调整订单失败！')
                })
                .finally(() => {
                    this.clearAdjustOrderData()
                })

        },

        handleHideAdjustOrder () {
            this.clearAdjustOrderData();
        },

        handleCancelOrder (orderData) {

            if ([0,1,3,4,5,6,8].indexOf(+orderData.status) !== -1) {
                return Promise.resolve(false)
            }

            return this.cancelOrder(orderData)
                .then(() => this.$message.success('撤单指令已发送！'))
                .catch(err => this.$message.error(err.message || '撤单指令发送失败！'))
        },

        getAdjustOrderTips (type, instrument_id, limit_price, limitPriceOld, volume, volumeOld) {
            if (type === 'volume') {
                return `确认调整： 商品 ${instrument_id}, 价格 ${limit_price}, 原未成交量 ${volumeOld}, 新设定交易量 ${volume}`
            } else {
                return `确认调整： 商品 ${instrument_id}, 原价格 ${limitPriceOld}, 新设定价格 ${limit_price}, 交易量 ${volume}`
            }
        },

        getAdjustOrderAccountResolved () {
            if (this.moduleType === 'account') {
                return this.currentId
            } else if (this.moduleType === 'strategy') {
                return this.adjustOrderForm.name
            }
        }, 

        cancelOrder (orderData) {
            const kungfuLocation = decodeKungfuLocation(orderData.source);
            const accountId = `${kungfuLocation.group}_${kungfuLocation.name}`;
            
            //撤单   
            if (this.moduleType === 'strategy') {
                return kungfuCancelOrder( orderData.orderId, accountId, this.currentId)
            } else if (this.moduleType === 'account') {
                return kungfuCancelOrder( orderData.orderId, accountId)
            }
        },

        makeOrder (moduleType, makeOrderForm, currentAccountResolved, strategyId) {
            if (moduleType === 'account') {
                return kungfuMakeOrder(makeOrderForm, currentAccountResolved)
            } else if (moduleType === 'strategy') {
                return kungfuMakeOrder(makeOrderForm, currentAccountResolved, strategyId)
            }
        },

        clearAdjustOrderData () {
            this.adjustOrderInputVisibility = false;
            this.adjustOrderInputSizeData = Object.freeze({});
            this.adjustOrderTargetData = Object({});
            this.adjustOrderForm = {
                name: '', // account_id in strategy
                instrument_id: '',
                instrument_type: '',
                exchange_id: '',
                limit_price: 0,
                volume: 0,
                side: 0,
                offset: 0,
                price_type: 0,
                hedge_flag: 0,
            };
            this.adjustOrderProp = '';
        }
    }
}