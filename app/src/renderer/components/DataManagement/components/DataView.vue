<!--
 * @Author: your name
 * @Date: 2020-06-22 14:15:01
 * @LastEditTime: 2020-06-22 14:16:38
 * @LastEditors: your name
 * @Description: In User Settings Edit
 * @FilePath: /kungfu/app/src/renderer/components/DataManagement/components/dataView.vue
--> 
<template>
    <tr-dashboard :noHeader="true">
        <el-tabs type="border-card" v-model="currentTabName" >
            <!-- <el-tab-pane label="概览">概览</el-tab-pane> -->
            <el-tab-pane label="数据明细" name="detail">
                <tr-table
                :data="detailDataList"
                :schema="schema"
                @dbclick="() => {}"
                >
                </tr-table>
            </el-tab-pane>
            <el-tab-pane label="可视化" name="charts">
                <div id="data-view-k-chart">

                </div>
            </el-tab-pane>
        </el-tabs>
    </tr-dashboard>
</template>

<script>
import { mapState } from 'vuex';
import { KF_DATASET_DIR } from '__gConfig/pathConfig';
import { kungfu } from '__gUtils/kungfuUtils';
import { dealQuote } from '__io/kungfu/watcher';

import { buildConfig } from '../chartConfig/kConfig';

const path = require('path');
const { echarts } = require('@/assets/js/static/echarts.min.js');

function calculateMA(dayCount, data) {
    var result = [];
    for (var i = 0, len = data.length; i < len; i++) {
        if (i < dayCount) {
            result.push('-');
            continue;
        }
        var sum = 0;
        for (var j = 0; j < dayCount; j++) {
            sum += data[i - j][1];
        }
        result.push((sum / dayCount).toFixed(2));
    }
    return result;
}

export default {
    props: {
        currentId: String
    },

    data () {

        this.detailDataCount = 0;
        this.detailDataCountLimit = 3000;

        this.schema = [{
                type: 'text',
                label: '数据生成时间',
                prop: "dataTime",
                width: '160px'
            }, 
            {
                type: 'text',
                label: '合约ID',
                prop: "instrumentId",
            }, 
            {
                type: 'text',
                label: '合约类型',
                prop: "instrumentType",
                width: '60px'
            }, 
            {
                type: 'text',
                label: '柜台ID',
                prop: "sourceId",
                width: '60px'
            }, 
            {
                type: 'text',
                label: '交易所ID',
                prop: "exchangeId",
                width: '65px'
            }, 
            {
                type: 'number',
                label: '收盘价',
                prop: "closePrice",
            }, 
            {
                type: 'number',
                label: '最高价',
                prop: "highPrice",
            },  
            {
                type: 'number',
                label: '最新价',
                prop: "lastPrice",
            }, 
            {
                type: 'number',
                label: '最低价',
                prop: "lowPrice",
            }, 
            {
                type: 'number',
                label: '跌停板价',
                prop: "lowerLimitPrice",
            }, 
            {
                type: 'number',
                label: '持仓量',
                prop: "openInterest",
            }, 
            {
                type: 'number',
                label: '今开盘价',
                prop: "openPrice",
            }, 
            {
                type: 'number',
                label: '昨收价',
                prop: "preClosePrice",
            }, 
            {
                type: 'number',
                label: '昨持仓量',
                prop: "preOpenInterest",
            }, 
            {
                type: 'number',
                label: '昨结价',
                prop: "preSettlementPrice",
            }, 
            {
                type: 'number',
                label: '结算价',
                prop: "settlementPrice",
            }, 
            {
                type: 'text',
                label: '交易日',
                prop: "tradingDay",
            }, 
            {
                type: 'number',
                label: '成交金额',
                prop: "turnover",
            }, 
            {
                type: 'number',
                label: '涨停板价',
                prop: "upperLimitPrice",
            }, 
            {
                type: 'number',
                label: '数量',
                prop: "volume",
            }
        ]

        return {
            currentTabName: 'detail',
            detailDataList: [],
        }
    },

    mounted () {
        this.loadData(this.currentDataSeriesId)
    },

    computed: {
        ...mapState({
            currentDataSeriesId: state => state.DATA_MANAGEMENT.currentDataSeriesId
        }),

        dataSeriesByInstrumentId () {
            let dataSeriesByInstrumentId = {};
            this.detailDataList.map(item => {
                const instrumentId = item.instrumentId;
                const sourceId = item.sourceId;
                const exchangeId = item.exchangeId;
                const mergedId = `${instrumentId}_${sourceId}_${exchangeId}`;

                if (!dataSeriesByInstrumentId[mergedId]) dataSeriesByInstrumentId[mergedId] = [];
                dataSeriesByInstrumentId[mergedId].push(item)
            })

            return dataSeriesByInstrumentId;
        },

        targetDataSource () {
            return this.dataSeriesByInstrumentId['000002_xtp_SZE']
                .sort((item1, item2) => item1.dataTimeNumber - item2.dataTimeNumber)
        },

        kData () {
            const targetDataSource = this.targetDataSource;
            return targetDataSource.map(item => {
                return +item.closePrice
            })
        },

        kVolume () {
            const targetDataSource = this.targetDataSource;
            return targetDataSource.map(item => {
                return item.volume
            })
        },

        kTime () {
            const targetDataSource = this.targetDataSource;
            return targetDataSource.map(item => {
                return item.dataTime
            })
        },

        kConfig () {
            return buildConfig(this.kTime, this.kData, this.kVolume)
        }
    },

    watch: {
        currentId (newVal) {
            if (newVal === this.currentDataSeriesId) {
                this.loadData(newVal)
            }
        },

        currentDataSeriesId (newVal) {
            this.refreshData();
            this.loadData(newVal)
        },

        currentTabName (tabName) {
            if (tabName === 'charts') {
                this.loadCharts()
            }
        }
    },

    methods: {

        async loadData (currentId) {
            if (!currentId) return false;
            const dataSetPath = path.join(KF_DATASET_DIR, currentId);
            const dataSource = kungfu.Assemble([ dataSetPath ])
            const dataList = await this.getDetailDataList(dataSource);
            this.detailDataList = Object.freeze(dataList.sort((item1, item2) => item2.dataTimeNumber - item1.dataTimeNumber))
        },

        loadCharts () {
            return this.$nextTick()
                .then(() => {
                    const dom = document.getElementById('data-view-k-chart');
                    if(!dom) return;
                    this.myChart = echarts.getInstanceByDom(dom)
                    if(this.myChart === undefined) this.myChart = echarts.init(dom);
                    this.myChart.setOption(this.kConfig)
                })
        },

        getDetailDataList (dataSource, dataList=[]) {
            
            if (this.detailDataCount > this.detailDataCountLimit) return Promise.resolve(dataList)
            this.detailDataCount += 1; 

            return this.getSeriesData(dataSource)
                .then(frame => {
                    if (frame === false) return dataList
                    if (frame !== null) {
                        const frameResolved = dealQuote(frame);
                        dataList.push({
                            ...frameResolved,
                            id: frameResolved.id + this.detailDataCount
                        })
                    }
                    return this.getDetailDataList(dataSource, dataList)
                })
        },

        getSeriesData (dataSource) {
            if (dataSource.dataAvailable()) {
                return this.$nextTick()
                    .then(() => {
                        const frame = dataSource.currentFrame().data();
                        dataSource.next();
                        if (frame.type === 'Quote') {
                            return frame
                        } else {
                            return null
                        }
                    })
            } else {
                return Promise.resolve(false)
            }
        },

        refreshData () {
            this.detailDataList = [];
            this.detailDataCount = 0;
            this.currentTabName = 'detail'
        },
    }
}
</script>

<style lang="scss">

    #pane-charts {
        height: 100%;
        width: 100%; 
    }

    #data-view-k-chart {
        height: 100%;
        width: 100%;
    }

</style>