<template>
    <tr-dashboard :noTitle="true">
        <div slot="dashboard-header"></div>
        <tr-table
        v-if="rendererTable"
        scrollerType="dynamic"
        :renderHtml="true"
        ref="logTable"
        :data="orderMap"
        :schema="schema"
        :sizeDependencies="['trades']"
        :renderCellClass="renderCellClass"
        ></tr-table>
    </tr-dashboard>
</template>

<script>

import { mapState } from 'vuex';

import { debounce } from '__gUtils/busiUtils'
import { dealTrade, dealOrder, dealOrderInput } from "__io/kungfu/watcher";

export default {
    props: {
        orders: {
            type: Array,
            default: () => ([])
        },

        trades: {
            type: Array,
            default: () => ([])
        },

        orderInputs: {
            type: Array,
            default: () => ([])
        },
    },

    data () {
          this.schema = [
            {
                label: 'orderInput',
                prop: 'orderInput',
            },
            {
                label: 'onOrder',
                prop: 'order',
            },
            
            {
                label: 'onTrade',
                prop: 'trades',
            },
        ];

        return {
            rendererTable: false,
            orderMap: [],
        }
    },

    mounted () {
        this.rendererTable = true;
        this.timer = setInterval(() => {
            const mapData = this.mergeMapByOrderId();
            this.orderMap = this.ResolveOrderMap(mapData);
        }, 3000)
    },

    computed: {
        ...mapState({
            processId: state => state.STRATEGY.currentStrategy.strategy_id
        }),

          processId: debounce(function (val) {
            this.resetData();
            if(!val) return;
            this.rendererTable = false;
            this.$nextTick().then(() => {
                this.rendererTable = true;
            })
        }, 100),
    },

    beforeDestroy() {
        clearInterval(this.timer);
    },

    methods: {
        mergeMapByOrderId () {
            let mapData = {};
         
            this.orderInputs.kfForEach(item => {
                const itemResolved = dealOrderInput(item);
                const { orderId, updateTime } = itemResolved;
                if (!mapData[orderId]) mapData[orderId] = {};
                mapData[orderId]['orderInput'] = itemResolved;  
                mapData[orderId]['id'] = orderId;  
                mapData[orderId]['ts'] = updateTime;  

            })

               this.orders.kfForEach(item => {
                const itemResolved = dealOrder(item);
                const { orderId } = itemResolved;
                if (!mapData[orderId]) return;
                mapData[orderId]['order'] = itemResolved;      
            })


            this.trades.kfForEach(item => {
                const itemResolved = dealTrade(item);
                const { orderId } = itemResolved;
                if (!mapData[orderId]) return;

                if (!(mapData[orderId] || {})['trades']) {
                    mapData[orderId]['trades'] = []
                } 

                mapData[orderId]['trades'].push(itemResolved);
            })

            return Object.freeze(mapData)
        },

        ResolveOrderMap (orderMap) {
            return Object.values(orderMap || {}).map(item => {
                const { id, orderInput, order, trades } = item;
                return {
                    id,
                    orderInput: this.turnOrderInputToLog(orderInput),
                    order: this.turnOrderToLog(order),
                    trades: this.turnTradesToLog(trades || [])
                }
            })
        },

        turnOrderInputToLog (orderInput) {
            const { orderId, priceType, limitPrice, side, offset, volume, hedgeFlag, accountId, exchangeId } = orderInput
            return `
                OrderId: ${orderId} <br/>
                PriceType: ${priceType} <br/>
                LimitPrice: ${limitPrice} <br/>
                Side: ${side} <br/>
                Offset: ${offset} <br/>
                HedgeFlag: ${hedgeFlag} <br/>
                volume: ${volume} <br/>
                Exchange: ${exchangeId} <br/>
                Account: ${accountId} <br/>
            `
        },

        turnOrderToLog (order) {
            const { orderId, priceType, limitPrice, side, offset, volume, hedgeFlag, accountId, exchangeId } = order
            return `
                OrderId: ${orderId} <br/>
                PriceType: ${priceType} <br/>
                LimitPrice: ${limitPrice} <br/>
                Side: ${side} <br/>
                Offset: ${offset} <br/>
                HedgeFlag: ${hedgeFlag} <br/>
                volume: ${volumeTraded}/${volume} <br/>
                Exchange: ${exchangeId} <br/>
                Account: ${accountId} <br/>
                Status: ${statusName} <br/>
            `       
        },

        turnTradesToLog (trades) {
            return trades.map(trade => JSON.stringify(trade)).join('\n')
        },

        resetData () {
            this.orderMap = [];
        },

        renderCellClass (prop, item) {

        }
    }

}
</script>

<style>

</style>