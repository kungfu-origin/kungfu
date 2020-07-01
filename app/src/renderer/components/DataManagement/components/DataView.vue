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
        <el-tabs type="border-card">
            <!-- <el-tab-pane label="概览">概览</el-tab-pane> -->
            <el-tab-pane label="数据明细">
                <tr-table
                :data="detailDataList"
                :schema="schema"
                @dbclick="() => {}"
                >
                </tr-table>
            </el-tab-pane>
            <el-tab-pane label="可视化">可视化</el-tab-pane>
        </el-tabs>
    </tr-dashboard>
</template>

<script>
import { mapState } from 'vuex';
import { KF_DATASET_DIR } from '__gConfig/pathConfig';
import { kungfu } from '__gUtils/kungfuUtils';
import { dealQuote } from '__io/kungfu/watcher';

const path = require('path');

export default {
    props: {
        currentId: String
    },

    data () {

        this.detailDataCount = 0;

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
                width: '70px'
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
            detailDataList: [],
        }
    },

    computed: {
        ...mapState({
            currentDataSeriesId: state => state.DATA_MANAGEMENT.currentDataSeriesId
        }),
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
        }
    },

    methods: {

        async loadData (currentId) {
            const dataSetPath = path.join(KF_DATASET_DIR, currentId);
            const dataSource = kungfu.Assemble([ dataSetPath ])
            const dataList = await this.getDetailDataList(dataSource);
            this.detailDataList = Object.freeze(dataList)
        },

        getDetailDataList (dataSource, dataList=[]) {
            
            if (this.detailDataCount > 1000) return Promise.resolve(dataList)
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
        },
    }
}
</script>

<style>

</style>