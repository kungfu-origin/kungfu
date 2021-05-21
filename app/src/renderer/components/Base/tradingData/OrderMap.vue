<template>
    <tr-dashboard :noTitle="true">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
            </tr-dashboard-header-item>
        </div>
        <tr-table
        v-if="afterRender"
        scrollerType="dynamic"
        :renderHtml="true"
        ref="logTable"
        :data="tableListAfterFilter"
        :schema="schema"
        :sizeDependencies="['trades', 'order']"
        ></tr-table>
    </tr-dashboard>
</template>

<script>

import { mapState } from 'vuex';

import { debounce, throttle } from '__gUtils/busiUtils'
import { dealOrderStat } from '__io/kungfu/watcher';

import baseMixin from '@/assets/mixins/baseMixin';

export default {
    mixins: [ baseMixin ],
    
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

        orderStat: {
            type: Object,
            default: () => ({})
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
        
        this.searchFilterKey = "orderInput, order, trades";

        return {
            tableList: [],
        }
    },

    mounted () {
        this.setOrderMap(this.processId);
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
            this.afterRender = false;
            this.$nextTick().then(() => {
                this.afterRender = true;
            })
        }, 100),

        orders () {
            this.setOrderMap(this.processId)
        },

        trades () {
            this.setOrderMap(this.processId)
        },

        orderInputs () {
            this.setOrderMap(this.processId)
        },
    },

    methods: {

        setOrderMap: throttle(function (processId) {
            const data = this.mergeMapByOrderId();
            if (processId !== this.processId) return;
            this.tableList = this.resolveOrderMap(data);
        }, 100),

        mergeMapByOrderId () {
            let mapData = {};
         
            this.orderInputs.kfForEach(item => {
                const itemResolved = item;
                const { orderId } = itemResolved;
                if (!mapData[orderId]) mapData[orderId] = {};
                mapData[orderId]['orderInput'] = Object.freeze(itemResolved);  
                mapData[orderId]['id'] = orderId;  
            })

            this.orders.kfForEach(item => {
                const itemResolved = item;
                const { orderId, updateTimeNum } = itemResolved;
                if (!mapData[orderId]) mapData[orderId] = {};
                mapData[orderId]['order'] = Object.freeze(itemResolved);      
                mapData[orderId]['id'] = orderId;  
                mapData[orderId]['updateTimeNum'] = updateTimeNum;  
            })


            this.trades.kfForEach(item => {
                let itemResolved = { ...item };
                const { orderId } = itemResolved;

                if (!mapData[orderId]) return;

                const orderStatByOrderId = dealOrderStat(this.orderStat[orderId] || null);
                //ctp trade返回的是交易所时间（xtp是自己维护），所用orderState内时间代替
                itemResolved.sourceUpdateTime = orderStatByOrderId.tradeTime;
                itemResolved.sourceUpdateTimeMMDD = orderStatByOrderId.tradeTimeMMDD;
                
                if (!(mapData[orderId] || {})['trades']) {
                    mapData[orderId]['trades'] = []
                } 

                mapData[orderId]['trades'].push(Object.freeze(itemResolved));
            })

            return Object.freeze(mapData)
        },

        resolveOrderMap (tableList) {
            return Object.freeze(Object.values(tableList || {})
            .sort((a, b) =>{
                return  b.updateTimeNum - a.updateTimeNum
            })
            .map(item => {
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
            if (!orderInput) return "";

            const { instrumentId, orderId, priceType, limitPrice, volume, hedgeFlag, accountId, exchangeId, updateTime } = orderInput
            return `
                ${updateTime} <br/>
                ${instrumentId} ${exchangeId} ${accountId} <br/>
                ${priceType} ${this.renderLine('side', orderInput)} ${this.renderLine('offset', orderInput)} ${hedgeFlag} <br/>
                价格: ${limitPrice} 量: ${volume} <br/>
                订单ID: ${orderId} <br/>
                <br/>
            `
        },

        turnOrderToLog (order) {
            if (!order) return "";

            const { instrumentId, orderId, priceType, limitPrice, volumeTraded, hedgeFlag, accountId, exchangeId, updateTime } = order
            return `
                ${updateTime} <br/>
                ${instrumentId} ${exchangeId} ${accountId} <br/>
                ${priceType} ${this.renderLine('side', order)} ${this.renderLine('offset', order)} ${hedgeFlag} <br/>
                价格: ${limitPrice} 量: ${volumeTraded} <br/>
                ${this.renderLine('status', order)}<br/>
                订单ID: ${orderId} <br/>
                <br/>
            `       
        },

        turnTradesToLog (trades) {
            if (!trades) return ""
            return trades.map(trade => `
                ${trade.updateTime} <br/>
                ${trade.sourceUpdateTime}（柜台时间） <br/>
                ${trade.instrumentId} ${trade.exchangeId} ${trade.accountId} <br/>
                ${this.renderLine('side', trade)} ${this.renderLine('offset', trade)} ${trade.hedgeFlag} <br/>
                价格: ${trade.price} 量: ${trade.volume} <br/>
                税: ${trade.tax} 手续费 ${trade.commission} <br/>
                订单ID: ${trade.orderId} <br/>
                
            `).join('<br/>') + "<br/>"
        },

        resetData () {
            this.tableList = [];
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
                    if (+item.status === 4) {
                        return `<span class="red">${item.statusName}</span>`
                    } else if ([3, 5, 6].indexOf(+item.status) !== -1) {
                        return `<span class="green">${item.statusName}</span>`
                    } else {
                        return item.statusName
                    }
            }
        }
    }

}
</script>

<style>

</style>