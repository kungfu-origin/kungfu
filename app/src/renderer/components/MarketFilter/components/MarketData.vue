<template>
    <div class="market-data__warp">
        <tr-dashboard title="" >
            <div slot="dashboard-header">
                <tr-dashboard-header-item v-if="proMode">
                    <span class="mouse-over" style="font-weight: 600; font-size: 11px" title="添加选股算法" @click="handleAddTask">选股</span>
                </tr-dashboard-header-item>
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
                    min-width="80"
                >
                </el-table-column>
                <el-table-column
                    label="交易所"
                    show-overflow-tooltip
                    min-width="60"
                >
                    <template slot-scope="props">
                        <span>{{ ExchangeIds[props.row.exchangeId] }}</span>
                    </template>
                </el-table-column>
                <el-table-column
                    label="开盘价"
                    show-overflow-tooltip
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                        <tr-blink-num
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'openPrice' )"
                        >
                        </tr-blink-num>
                    </template>
                </el-table-column>
                <el-table-column
                    label="收盘价"
                    show-overflow-tooltip
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                        <tr-blink-num
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'closePrice' )"
                        >
                        </tr-blink-num>
                    </template>
                </el-table-column>
                <el-table-column
                    label="最高价"
                    show-overflow-tooltip
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                        <tr-blink-num
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'highPrice' )"
                        >
                        </tr-blink-num>
                    </template>
                </el-table-column>
                <el-table-column
                    label="最低价"
                    show-overflow-tooltip
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                        <tr-blink-num
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="getValueFromMarketData(props.row, 'lowPrice' )"
                        >
                        </tr-blink-num>
                    </template>
                </el-table-column>
                <el-table-column
                    label="最新价"
                    show-overflow-tooltip
                    class-name="blink-cell"
                    align="right"
                >
                    <template slot-scope="props">
                        <tr-blink-num
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"     
                        :num="getValueFromMarketData(props.row, 'lastPrice' )"
                        >
                        </tr-blink-num>
                    </template>
                </el-table-column>
                    <el-table-column
                    label="当日成交量"
                    show-overflow-tooltip
                    class-name="blink-cell"
                    align="right"
                    min-width="100"
                >
                    <template slot-scope="props">
                        <tr-blink-num
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"     
                        :num="getValueFromMarketData(props.row, 'volume' )"
                        >
                        </tr-blink-num>
                    </template>
                </el-table-column>
                <el-table-column
                    label="7日均成交量"
                    show-overflow-tooltip
                    class-name="blink-cell"
                    align="right"
                    min-width="100"
                >
                    <template slot-scope="props">
                        <tr-blink-num
                        :theKey="`${currentTickerSet.name}_${props.row.instrumentId}`"   
                        :num="marketAvgVolume7Days[`${props.row.instrumentId}_${props.row.exchangeId}`] || '--'"
                        >
                        </tr-blink-num>
                    </template>
                </el-table-column>
                <el-table-column
                    label=""
                    align="right"
                    min-width="80"
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

import { mapGetters, mapState } from 'vuex';

import { getIndexFromTargetTickers } from '__gUtils/busiUtils';
import { dealQuote } from '__io/kungfu/watcher';
import { ExchangeIds } from "kungfu-shared/config/tradingConfig";

import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';
import taskMixin from '@/components/Task/js/taskMixin';


export default {

    mixins: [ tickerSetMixin, taskMixin ],

    props: {

        marketData: {
            type: Object,
            default: () => ({})
        }
    },

    data () {
        this.ExchangeIds = ExchangeIds;
        return {}
    },

    computed: {
        ...mapState({
            tdAccountSource: state => state.BASE.tdAccountSource || {},
            marketAvgVolume: state => state.MARKET.marketAvgVolume || {},
        }),

        ...mapGetters([
            "proMode"
        ]),

        marketAvgVolume7Days () {
            return this.marketAvgVolume[7] || {}
        }
    },

    methods: {

        handleAddTask () {
            this.$bus.$emit('set-task', {
                type: "mFilter", 
                initData: {
                    'tickerSet': this.currentTickerSetName
                }
            })
        },

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