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
        :sizeDependencies="['trades', 'order']"
        ></tr-table>
    </tr-dashboard>
</template>

<script>

import { mapState } from 'vuex';

import { debounce, throttle } from '__gUtils/busiUtils'
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
                label: '下单信息',
                prop: 'orderInput',
            },
            {
                label: '委托返回',
                prop: 'order',
            },
            
            {
                label: '成交返回',
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
        this.setOrderMap();
    },

    computed: {
        ...mapState({
            processId: state => state.STRATEGY.currentStrategy.strategy_id
        }) 
    },

    watch: {
        processId: debounce(function (val) {
            this.resetData();
            if(!val) return;
            this.rendererTable = false;
            this.$nextTick().then(() => {
                this.rendererTable = true;
            })
        }, 100),

        orders () {
            this.setOrderMap()
        },

        trades () {
            this.setOrderMap()
        },

        orderInputs () {
            this.setOrderMap()
        },
    },

    methods: {

        setOrderMap: throttle(function () {
            const mapData = this.mergeMapByOrderId();
            this.orderMap = this.resolveOrderMap(mapData);
        }, 500),

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

        resolveOrderMap (orderMap) {
            return Object.freeze(Object.values(orderMap || {}).map(item => {
                const { id, orderInput, order, trades } = item;
                return Object.freeze({
                    id,
                    orderInput: this.turnOrderInputToLog(orderInput),
                    order: this.turnOrderToLog(order),
                    trades: this.turnTradesToLog(trades || [])
                })
            }));
        },

        turnOrderInputToLog (orderInput) {
            const { instrumentId, orderId, priceType, limitPrice, volume, hedgeFlag, accountId, exchangeId, updateTime } = orderInput
            return `
                Time: ${updateTime} <br/>
                InstrumentId: ${instrumentId} <br/>
                PriceType: ${priceType} <br/>
                LimitPrice: ${limitPrice} <br/>
                Side: ${this.renderLine('side', orderInput)} <br/>
                Offset: ${this.renderLine('offset', orderInput)} <br/>
                HedgeFlag: ${hedgeFlag} <br/>
                volume: ${volume} <br/>
                Exchange: ${exchangeId} <br/>
                Account: ${accountId} <br/>
                OrderId: ${orderId} <br/>
                <br/>
            `
        },

        turnOrderToLog (order) {
            const { instrumentId, orderId, priceType, limitPrice, volumeTraded, hedgeFlag, accountId, exchangeId, updateTime } = order
            return `
                Time: ${updateTime} <br/>
                InstrumentId: ${instrumentId} <br/>
                PriceType: ${priceType} <br/>
                LimitPrice: ${limitPrice} <br/>
                Side: ${this.renderLine('side', order)} <br/>
                Offset: ${this.renderLine('offset', order)} <br/>
                HedgeFlag: ${hedgeFlag} <br/>
                volume: ${volumeTraded} <br/>
                Exchange: ${exchangeId} <br/>
                Account: ${accountId} <br/>
                OrderId: ${orderId} <br/>
                Status: ${this.renderLine('status', order)} <br/>
                <br/>
            `       
        },

        turnTradesToLog (trades) {
            return trades.map(trade => `
                Time: ${trade.updateTime} <br/>
                Price: ${trade.price} Volume: ${trade.volume} <br/>
            `).join('<br>')
        },

        resetData () {
            this.orderMap = [];
        },

        renderLine (prop, item) {
            switch (prop) {
                case "side":
                    if (item.side === "买") {
                        return `<span class="red">${item.side}</span>`
                    } else {
                        return `<span class="green">${item.side}</span>`
                    }
                case "offset":
                    if (item.offset.includes("平")) {
                        return `<span class="green">${item.offset}</span>`
                    } else {
                        return `<span class="red">${item.offset}</span>`
                    }
                case "status":
                    if (+item.status !== 4) {
                        return item.statusName
                    } else {
                        return `<span class="red">${item.statusName}</span>`
                    }
            }
        }
    }

}
</script>

<style>

</style>