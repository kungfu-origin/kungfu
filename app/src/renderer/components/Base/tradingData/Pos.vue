<template>
<tr-dashboard :title="`持仓 ${currentTitle}`">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-refresh mouse-over" title="刷新" @click="handleRefresh"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-download mouse-over" title="导出" @click="handleExport"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-button size="mini" @click="makeOrderDialogVisiblity = true">下单</el-button>
        </tr-dashboard-header-item>
    </div>
    <tr-table
        v-if="rendererTable"
        :data="tableData"
        :schema="schema"
        :renderCellClass="renderCellClass"
    ></tr-table>

    <make-order-dialog
        v-if="makeOrderDialogVisiblity"
        :visible.sync="makeOrderDialogVisiblity"
        :moduleType="moduleType"
        :currentId="currentId"
    >
    </make-order-dialog>
</tr-dashboard>
</template>

<script>

import MakeOrderDialog from '../MakeOrderDialog';
import tradingDataMixin from './js/tradingDataMixin';

import { debounce } from '__gUtils/busiUtils';
import { dealPos } from '__gUtils/kungfuUtils';
import { writeCSV } from '__gUtils/fileUtils';


const ls = require('local-storage');

export default {
    name: 'positions',
  
    mixins: [ tradingDataMixin ],

    data() {
        return {
            tableData: Object.freeze([]),
            makeOrderDialogVisiblity: false
        }
    },

    computed:{
        schema() {
            const t = this
            return [{
                    type: 'text',
                    label: '代码',
                    prop: 'instrumentId',
                    width: '60px'
                },{
                    type: 'text',
                    label: '多空',
                    prop: 'direction',
                    width: '50px'
                },{
                    type: 'number',
                    label: '昨',
                    prop: 'yesterdayVolume',
                    flex: 1
                },{
                    type: 'number',
                    label: '今',
                    prop: 'todayVolume',
                    flex: 1
                },{
                    type: 'number',
                    label: '总',
                    prop: 'totalVolume',
                    flex: 1
                },{
                    type: 'number',
                    label: '开(持)仓均价',
                    prop: 'avgPrice',
                    flex: 1.2
                },{
                    type: 'number',
                    label: '最新价',
                    prop: 'lastPrice',
                    flex: 1.2
                },{
                    type: 'number',
                    label: '浮动盈亏',
                    prop: 'unRealizedPnl',
                    flex: 1.5
                }
            ]
        }
    },

    components: {
        MakeOrderDialog
    },


    watch: {
        kungfuData (positions) {
            const t = this;
            const positionsResolve = t.dealPositionList(positions, t.searchKeyword);
            (positionsResolve.length) && (t.tableData = positionsResolve);
        }
    },

    destroyed(){
        this.saveInstrumentIdsToLS();
    },
    
    methods:{
        handleExport () {
            const t = this;
            t.$saveFile({
                title: '保存持仓信息',
            }).then(filename => {
                if(!filename) return;
                writeCSV(filename, t.tableData)
            })
        },

        dealPositionList (positions, searchKeyword) {
            const t = this;
            let positionDataByKey = {};

            let positionsAfterFilter = positions
            .filter(item => {
                if (searchKeyword.trim() === '') return true;
                const { instrument_id } = item
                return instrument_id.includes(searchKeyword) 
            })

            if (t.moduleType === 'strategy') {
                positionsAfterFilter = positionsAfterFilter.filter(item => Number(item.update_time) >= t.addTime )
            }

            if (!positionsAfterFilter.length) return Object.freeze([]);

            positionsAfterFilter.kfForEach(item => {
                let positionData = dealPos(item);
                positionData.update = true;
                const poskey = t.getKey(positionData)
                positionDataByKey[poskey] = positionData;
            })

            return Object.freeze(Object.values(positionDataByKey).sort((a, b) =>{
                return a.instrumentId - b.instrumentId
            }))
        },

        //拼接key值
        getKey(data) {
            return (data.instrumentId + data.direction)
        },

        saveInstrumentIdsToLS () {
            if(!this.tableData.length) return;
            const instrumentIdsList = ls.get('instrument_ids_list')
            const instrumentIds = this.tableData
                .map(item => item.instrumentId)
                .reduce((result, item) => {
                    if (typeof result !== "object") {
                        return {
                            [result]: 1,
                            [item]: 1
                        }
                    }
                    result[item] = 1;
                    return result;
                })
            
            ls.set('instrument_ids_list', {
                ...instrumentIdsList,
                ...instrumentIds
            })
        }
    }
}
</script>

<style lang="scss">
.positions{
    height: 100%;
}
</style>