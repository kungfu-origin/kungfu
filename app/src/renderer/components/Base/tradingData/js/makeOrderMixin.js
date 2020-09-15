
import { kungfuCancelOrder, kungfuMakeOrder } from '__io/kungfu/makeCancelOrder';

export default {

    data () {

        return {
            //adjustOrder
            adjustOrderInputVisibility: false, 
            adjustOrderInputData: Object.freeze({}),
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
            
            this.adjustOrderInputData = Object.freeze({
                left,
                top,
                width,
                height
            })

            this.adjustOrderProp = cell.prop;
            this.adjustOrderTargetData = Object.freeze(row);

            //build form
            this.$set(this.adjustOrderForm, 'name', row.accountId)
            this.$set(this.adjustOrderForm, 'instrument_id', row.instrumentId)
            this.$set(this.adjustOrderForm, 'exchange_id', row.exchangeId)
            this.$set(this.adjustOrderForm, 'limit_price', row.limitPrice)
            this.$set(this.adjustOrderForm, 'volume', row.volumeLeft)
            this.$set(this.adjustOrderForm, 'volumeLeft', row.volumeLeft)
            this.$set(this.adjustOrderForm, 'side', row.sideOrigin)
            this.$set(this.adjustOrderForm, 'offset', row.offsetOrigin)
            this.$set(this.adjustOrderForm, 'price_type', row.priceTypeOrigin)
            this.$set(this.adjustOrderForm, 'hedge_flag', row.hedgeFlagOrigin)

            this.$nextTick()
                .then(() => {
                    this.adjustOrderInputVisibility = true;
                })
        },

        handleBlurAdjustOrderInput () {
            const { instrument_id, limit_price, volume, volumeLeft } = this.adjustOrderForm;

            this.$confirm(
                `确认调整： 商品 ${instrument_id}, 价格 ${limit_price}, 原未交易量 ${volumeLeft}, 新设定交易量 ${volume}`, 
                '提示', 
                {
                    confirmButtonText: '确 定', 
                    cancelButtonText: '取 消'
                })
                .then(() => {
                    return this.handleCancelOrder(this.adjustOrderTargetData)
                        .then(() => this.makeOrder(
                                this.moduleType, 
                                this.adjustOrderForm, 
                                this.getAdjustOrderAccountResolved(), 
                                this.currentId
                        ))
                        .then(() => this.$message.success('调仓指令发送成功！'))
                })
                .catch((err) => {
                    if(err == 'cancel') return
                    this.$message.error(err.message || '操作失败！')
                })

        },

        handleHideAdjustOrder () {
            this.adjustOrderInputVisibility = false;
            this.adjustOrderInputData = Object.freeze({})
        },

        handleCancelOrder (orderData) {
            const kungfuLocation = decodeKungfuLocation(orderData.source);
            const accountId = `${kungfuLocation.group}_${kungfuLocation.name}`;
            const gatewayName = `td_${accountId}`;

            if(this.processStatus[gatewayName] !== 'online') {
                this.$message.warning(`需要先启动 TD ${accountId} 交易进程！`)
                return;
            }
            
            //撤单   
            if (this.moduleType === 'strategy') {
                return kungfuCancelOrder( orderData.orderId, accountId, this.currentId)
                    .then(() => this.$message.success('撤单指令已发送！'))
                    .catch(err => this.$message.error(err.message || '撤单指令发送失败！'))
            } else if (this.moduleType === 'account') {
                return kungfuCancelOrder( orderData.orderId, accountId)
                    .then(() => this.$message.success('撤单指令已发送！'))
                    .catch(err => this.$message.error(err.message || '撤单指令发送失败！'))
            }
        },

        getAdjustOrderAccountResolved () {
            if (this.moduleType === 'account') {
                return this.currentId
            } else if (this.moduleType === 'strategy') {
                return this.adjustOrderForm.name
            }
        }, 

        makeOrder (moduleType, makeOrderForm, currentAccountResolved, strategyId) {
            if (moduleType === 'account') {
                return kungfuMakeOrder(makeOrderForm, currentAccountResolved)
                    .then(() => this.$message.success('下单指令已发送！'))
                    .catch(err => this.$message.error(err))
            } else if (moduleType === 'strategy') {
                return kungfuMakeOrder(makeOrderForm, currentAccountResolved, strategyId)
                    .then(() => this.$message.success('下单指令已发送！'))
                    .catch(err => this.$message.error(err))
            }
        },
    }
}