<template>
<tr-dashboard :title="noTitle ? '' : `持仓 ${currentTitle}`">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-download mouse-over" title="导出" @click="handleExport"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="moduleType === 'strategy'">
            <el-button size="mini" @click="$emit('showMakeOrderDashboard')">下单</el-button>
        </tr-dashboard-header-item>
    </div>
    <tr-table
        v-if="rendererTable"
        :data="tableData"
        :schema="schema"
        :keyField="moduleType === 'ticker' ? 'accountIdResolved' : 'id'"
        :renderCellClass="renderCellClass"
        :isActiveFunc="isActiveTicker"
        @clickCell="(e, item) => handleClickCell(item)"
    ></tr-table>
</tr-dashboard>
</template>

<script>

import tradingDataMixin from './js/tradingDataMixin';

import { writeCSV } from '@kungfu-trader/kungfu-uicc/utils/fileUtils';
import { toDecimal, deepClone } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import { posHeader } from '@/components/Base/tradingData/js/tableHeaderConfig';
import { sendDataToDaemonByPm2 } from "@kungfu-trader/kungfu-uicc/utils/processUtils";

export default {
    name: 'positions',
  
    mixins: [ tradingDataMixin ],

    props: {
        name: {
            type: String,
            default: ''
        },

        currentTicker: {
            type: Object,
            default: () => null
        }
    },

    data () {
        return {
            subscribedTickersStr: "",
            hasInitOrderBook: false,
            tableData: Object.freeze([]),
            dataByKey: Object.freeze({})
        }
    },

    computed: {
        schema() {
            return posHeader(this.moduleType)
        },

        isTickerModule () {
            if (this.currentTicker && this.currentTicker.instrumentId) return true
            return false;
        },
    },

    watch: {
        kungfuData (positions) {
            const positionsResolved = this.dealPositionList(positions, this.searchKeyword) || {};
            const dataList = positionsResolved.dataList || [];
            this.tableData = dataList;
            this.dataByKey = positionsResolved.dataByKey || {};

            //订阅行情
            const subscribePosTickers = dataList.map(item => {
                return {
                    instrumentId: item.instrumentId,
                    exchangeId: item.exchangeId,
                    source: item.sourceId
                }
            })
            const subscribedTickersStr = subscribePosTickers.slice(0)
                .sort((item1, item2) => {
                    const id1 = `${item1.instrumentId}_${item1.exchangeId}`;
                    const id2 = `${item2.instrumentId}_${item2.exchangeId}`;
                    return id1.localeCompare(id2);
                })

            if (this.subscribedTickersStr !== subscribedTickersStr) {
                sendDataToDaemonByPm2('MAIN_RENDERER_SUBSCRIBED_TICKERS', subscribePosTickers)
                this.subscribedTickersStr = subscribedTickersStr;
            }

            //更新orderbook
            if (!this.hasInitOrderBook && this.tableData.length) {
                this.$bus.$emit('orderbook-tickerId', {
                    instrumentId: this.tableData[0].instrumentId,
                    exchangeId: this.tableData[0].exchangeId
                });
                this.hasInitOrderBook = true;
            }
        },

        currentId () {
            this.hasInitOrderBook = false;
        }
    },

    methods: {

        handleClickCell (item) {
            if (this.isTickerModule) {
                this.$emit('activeTicker', item)
                //ticker mode need delete default accountId
                let itemResolved = deepClone(item);
                delete itemResolved.accountIdResolved;
                this.$emit('makeOrder', itemResolved)
            } else {
                this.$emit('makeOrder', item)
            };

            this.$bus.$emit('orderbook-tickerId', {
                instrumentId: item.instrumentId,
                exchangeId: item.exchangeId
            });
        },

        handleExport () {
            this.$saveFile({
                title: '保存持仓信息',
            }).then(filename => {
                if(!filename) return;
                writeCSV(filename, this.tableData)
            })
        },

        dealPositionList (positions, searchKeyword) {
            let positionDataByKey = {};
            let positionsAfterFilter = positions
                .filter(item => !!Number(item.totalVolume))
                .filter(item => {
                    if (searchKeyword.trim() === '') return true;
                    const { instrumentId } = item;
                    return instrumentId.includes(searchKeyword);
                })

            if (this.moduleType === 'strategy') {
                positionsAfterFilter = positionsAfterFilter.filter(item => item.updateTimeNum >= BigInt(this.addTime));
            }


            if (!positionsAfterFilter.length) return {
                dataByKey: Object.freeze({}),
                dataList: Object.freeze([])
            };

            positionsAfterFilter.kfForEach(item => {
                let positionData = { ...item };
                positionData.update = true;
          
                positionData.yesterdayVolume = toDecimal(item.yesterdayVolume); 
                positionData.todayVolume = toDecimal(item.todayVolume); 
                positionData.totalVolume = toDecimal(item.totalVolume); 
                positionData.avgPrice = toDecimal(item.avgPrice); 
                positionData.lastPrice = toDecimal(item.lastPrice); 
                positionData.totalPrice = toDecimal(item.totalPrice); 
                positionData.totalMarketPrice = toDecimal(item.totalMarketPrice); 
                positionData.unRealizedPnl = toDecimal(item.unRealizedPnl); 

                const poskey = this.getKey(positionData);
                positionDataByKey[poskey] = Object.freeze(positionData);
            })

            return {
                dataByKey: Object.freeze(positionDataByKey),
                dataList: Object.freeze(Object.values(positionDataByKey).sort((a, b) => {
                    const result = a.instrumentId.localeCompare(b.instrumentId);
                    return result === 0 ? (a.directionOrigin || '').toString().localeCompare((b.directionOrigin || '').toString()) : result;
                }))
            };
        },

        //拼接key值
        getKey (data) {
            if (this.moduleType === 'ticker') {
                return data.accountIdResolved
            }
            return `${data.instrumentId}_${data.directionOrigin}`
        },

        isActiveTicker (item) {
            if (!this.isTickerModule) return false

            const key = this.getKey(item);
            const keyOfCurrentTicker = this.getKey(this.currentTicker)
            if (key === keyOfCurrentTicker) return true;
            return false;
        }
    }
}
</script>

<style lang="scss">
.positions{
    height: 100%;
}
</style>