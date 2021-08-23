<template>
    <div class="market-data__warp">
        <tr-dashboard title="" >
            <div slot="dashboard-header">
                <tr-dashboard-header-item v-if="proMode">
                    <span class="mouse-over text-icon" title="添加选股算法" @click="handleAddTask">选股</span>
                </tr-dashboard-header-item>
                <tr-dashboard-header-item>
                    <span class="mouse-over text-icon" title="订阅" @click="handleSubscribeAllTickers">订阅</span>
                </tr-dashboard-header-item>
                <tr-dashboard-header-item>
                    <el-select class="dashboard-header-selector" :value="currentTickerSetName" @change="handleSetTickerSet">
                        <el-option 
                            v-for="tickerSet in tickerSets" 
                            :key="tickerSet.name"    
                            :label="`当前标的池：${tickerSet.name || ''}`"
                            :value="tickerSet.name">
                        </el-option>
                    </el-select>
                </tr-dashboard-header-item>
                <tr-dashboard-header-item>
                    <el-button size="mini" @click="tickerSetsManagerDialogVisiblity = true" title="设置标的池">设置标的池</el-button>
                </tr-dashboard-header-item>
                <tr-dashboard-header-item>
                    <el-button size="mini" @click="handleAddTicker('marketdata')" title="添加">添加标的</el-button>
                </tr-dashboard-header-item>
            </div>
            <el-table
            size="small"
            :data="currentTickerSetTickersResolved || []"
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
                        <span class="tr-oper-delete" @click.stop="handleDeleteTicker(props.row, currentTickerSet)"><i class=" el-icon-delete mouse-over" title="删除标的"></i></span>
                    </template>
                </el-table-column>
            </el-table>
        </tr-dashboard>

        <AddTickerDialog
            v-if="addTickerDialogVisiblity"
            :visible.sync="addTickerDialogVisiblity"
            @confirm="tickerData => handleAddTickerConfirm(tickerData, addTickerTargetTickerSetName)"
        >
        </AddTickerDialog>  

        <TickerSetsManagerDialog
            v-if="tickerSetsManagerDialogVisiblity"
            :visible.sync="tickerSetsManagerDialogVisiblity"
            @addTicker="editingTickerSet => handleAddTicker('tickerSetManager', editingTickerSet)"
        ></TickerSetsManagerDialog>
    </div>
</template>

<script>

import { mapGetters, mapState } from 'vuex';

import { findTargetFromArray } from '__gUtils/busiUtils';
import { ExchangeIds } from "@kungfu-trader/kungfu-uicc/config/tradingConfig";

import TickerSetsManagerDialog from '@/components/MarketFilter/components/TickerSetsManagerDialog';
import AddTickerDialog from '@/components/MarketFilter/components/AddTickerDialog';

import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';
import taskMixin from '@/components/Task/js/taskMixin';

export default {

    mixins: [ tickerSetMixin, taskMixin ],

    props: {

        marketData: {
            type: Object,
            default: () => ({})
        },

        currentId: String,

        moduleType: String
    },

    data () {
        this.ExchangeIds = ExchangeIds;
        return {
            addTickerDialogVisiblity: false,
            addTickerType: '',
            addTickerTargetTickerSetName: '',

            tickerSetsManagerDialogVisiblity: false,
        }
    },

    components: {
        TickerSetsManagerDialog,
        AddTickerDialog
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

        handleAddTicker (type, targetTickerSet = null) {
            const targetTickerSetResolved = Object.freeze(targetTickerSet || this.currentTickerSet);
            if (targetTickerSetResolved === null || Object.keys(targetTickerSetResolved).length === 0) {
                this.$message.warning('请先添加标的池！')
                return;
            }
            this.addTickerType = type;
            this.addTickerTargetTickerSetName = Object.freeze(targetTickerSet || this.currentTickerSet || {}).name || '';
            this.addTickerDialogVisiblity = true;
        },

        handleSubscribeAllTickers () {
            this.subscribeAllTickers(false)
        },

        handleSetTickerSet (tickerSetName) {
            const targetOriginTickerSet = findTargetFromArray(this.tickerSets, 'name', tickerSetName)
            this.handleSetCurrentTickerSet(targetOriginTickerSet)
        },

        handleAddTask () {
            this.$bus.$emit('set-task', {
                type: "mFilter", 
                initData: {
                    'tickerSet': this.currentTickerSetName
                }
            })
        },

        handleRowClick (row) {
            const quoteData = this.getMarketData(row);
            this.$bus.$emit('update:make-order', {
                currentId: this.currentId,
                moduleType: this.moduleType,
                orderInput: {
                    ...quoteData,
                    instrumentType: (quoteData || '').instrumentTypeOrigin || 0
                }
            })

            this.$bus.$emit("orderbook-tickerId", {
                instrumentId: quoteData.instrumentId,
                exchangeId: quoteData.exchangeId
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
            const { exchangeId, instrumentId} = tickerData;
            const id = `${exchangeId}_${instrumentId}`;
            const target = this.marketData[id] || null;
            return Object.freeze(target);
        }
    }
}
</script>

<style lang="scss">

.market-data__warp {
    height: 100%;
}

</style>