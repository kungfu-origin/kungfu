<template>
    <div class="market-data__warp">
        <tr-dashboard title="" >
            <div slot="dashboard-header">
                <tr-dashboard-header-item>
                    <el-button size="mini" @click="handleAddTicker" title="添加">添加</el-button>
                </tr-dashboard-header-item>
            </div>
            <el-table
            size="small"
            :data="currentTickerSetTickers || []"
            height="100%"
            @row-click="handleRowClick"
            >

                <el-table-column
                    prop="instrumentId"
                    label="标的"
                    show-overflow-tooltip
                    min-width="60"
                >
                </el-table-column>
                <el-table-column
                    label="开盘价"
                    show-overflow-tooltip
                    min-width="60"
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                        <BlinkNum
                        :className="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'openPrice' )"
                        >
                        </BlinkNum>
                    </template>
                </el-table-column>
                <el-table-column
                    label="收盘价"
                    show-overflow-tooltip
                    min-width="60"
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                            <BlinkNum
                        :className="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'closePrice' )"
                        >
                        </BlinkNum>
                    </template>
                </el-table-column>
                <el-table-column
                    label="最高价"
                    show-overflow-tooltip
                    min-width="60"
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                            <BlinkNum
                        :className="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'highPrice' )"
                        >
                        </BlinkNum>
                    </template>
                </el-table-column>
                <el-table-column
                    label="最低价"
                    show-overflow-tooltip
                    min-width="60"
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                            <BlinkNum
                        :className="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'lowPrice' )"
                        >
                        </BlinkNum>
                    </template>
                </el-table-column>
                <el-table-column
                    label="最新价"
                    show-overflow-tooltip
                    min-width="60"
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                            <BlinkNum
                        :className="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"     
                        :num="getValueFromMarketData(props.row, 'lastPrice' )"
                        >
                        </BlinkNum>
                    </template>
                </el-table-column>
                    <el-table-column
                    label="当日成交量"
                    show-overflow-tooltip
                    min-width="60"
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                            <BlinkNum
                        :className="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"     
                        :num="getValueFromMarketData(props.row, 'volume' )"
                        >
                        </BlinkNum>
                    </template>
                </el-table-column>
                <el-table-column
                    label="7日均成交量"
                    show-overflow-tooltip
                    min-width="60"
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                        <BlinkNum
                        :className="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        num="--"
                        >
                        </BlinkNum>
                    </template>
                </el-table-column>
                <el-table-column
                    label=""
                    align="right"
                    min-width="40"
                >
                    <template slot-scope="props">
                        <span class="tr-oper-delete" @click.stop="handleDeleteTicker(props.row)"><i class=" el-icon-delete mouse-over" title="删除标的"></i></span>
                    </template>
                </el-table-column>
            </el-table>
        </tr-dashboard>


        <AddSetTickerSetDialog
            v-if="addSetTickerSetDialogVisiblity"
            :visible.sync="addSetTickerSetDialogVisiblity"
            @addTicker="handleAddTicker"
            @confirm="handleConfirmAddSetTickerSet"
            
        ></AddSetTickerSetDialog>

        <AddTickerDialog
            v-if="addTickerDialogVisiblity"
            :visible.sync="addTickerDialogVisiblity"
            @confirm="handleAddTickerConfirm"
        >
        </AddTickerDialog>  
    </div>
</template>

<script>

import { mapState } from 'vuex';

import BlinkNum from './BlinkNum'; 

import { getIndexFromTargetTickers } from '__gUtils/busiUtils';
import { dealQuote } from '__io/kungfu/watcher';

import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';


export default {

    mixins: [ tickerSetMixin ],

    props: {

        marketData: {
            type: Object,
            default: () => ({})
        }
    },

    computed: {
        ...mapState({
            tdAccountSource: state => state.BASE.tdAccountSource || {},
        })
    },

    components: {
        BlinkNum
    },

    methods: {

        handleRowClick (row) {
            this.$emit('clickQuote', this.getMarketData(row))
        },
        
        handleAddTicker () {
            this.addTickerDialogVisiblity = true;
            this.bindAddTickerToTickerSet();
        },

        handleDeleteTicker (ticker) {
            const { name, tickers } = this.currentTickerSet;
            const targetIndex = getIndexFromTargetTickers(tickers, ticker)

            if (targetIndex !== -1) {
                let targetTickers = tickers.slice(0)
                targetTickers.splice(targetIndex, 1);
                this.handleConfirmAddSetTickerSet({
                    name, 
                    tickers: targetTickers
                })
            }
        },

        bindAddTickerToTickerSet () {
            this.$bus.$off('add-ticker-for-ticker-set')
            this.$bus.$on('add-ticker-for-ticker-set', (tickerData) => {
                const { name, tickers } = this.currentTickerSet;
                const targetIndex = getIndexFromTargetTickers(tickers, tickerData)
                let newTickers = tickers.slice(0);
                
                if (targetIndex === -1) {
                    newTickers.push(tickerData)
                } else {
                    newTickers.splice(targetIndex, 1, tickerData)
                }

                this.handleConfirmAddSetTickerSet({
                    name,
                    tickers: newTickers
                })
            })
        },

        getValueFromMarketData (tickerData, key) {
            const quote = this.getMarketData(tickerData);
            if (!quote) {
                return '--'
            } else {
                return quote[key]
            }
        },

        getMarketData (tickerData) {
            const { exchangeId, instrumentId, source } = tickerData;
            const id = `${exchangeId}_${instrumentId}_${source}`;
            const target = this.marketData[id] || null;
            if (target) {
                return dealQuote(target)
            } else {
                return target                
            }
        }
    }
}
</script>

<style lang="scss">

.market-data__warp {
    height: 100%;
}

</style>