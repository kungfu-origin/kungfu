<template>
    <div class="kf-ticker-set__warp">
        <tr-dashboard title="标的池列表">
            <div slot="dashboard-header">
                <tr-dashboard-header-item>
                    <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
                </tr-dashboard-header-item>
                <tr-dashboard-header-item>
                    <span class="mouse-over" style="font-weight: 600; font-size: 11px" title="订阅" @click="handleSubscribeAllTickers">订</span>
                </tr-dashboard-header-item>
                <tr-dashboard-header-item>
                    <el-button size="mini" @click="handleAddTickerSet" title="添加">添加</el-button>
                </tr-dashboard-header-item>
            </div>
            <div class="table-body ticker-set-table">
                <el-table
                    size="small"
                    :data="tableListAfterFilter"
                    height="100%"
                    @row-click="handleClickRow"
                    :row-class-name="handleSelectRow"
                >
                    <el-table-column
                        prop="name"
                        label="名称"
                        show-overflow-tooltip
                    ></el-table-column>
                    <el-table-column
                        prop="tickers"
                        label="内含标的"
                        show-overflow-tooltip
                    >
                        <template slot-scope="props">
                            {{ (props.row.tickers || []).map(ticker => ticker.instrumentId).join(', ')  }}
                        </template>
                    </el-table-column>
                    <el-table-column
                        label=""
                        align="right"
                        width="80px"
                    >
                    <template slot-scope="props">
                        <span class="tr-oper"><i class="el-icon-setting mouse-over" title="设置" @click.stop="handleSetTickerSet(props.row)"></i></span>
                        <span class="tr-oper-delete"><i class=" el-icon-delete mouse-over" title="删除" @click.stop="handleRemoveTickerSet(props.row)"></i></span>
                    </template>
                    </el-table-column>
                </el-table>
            </div>
        </tr-dashboard>

        <AddSetTickerSetDialog
            v-if="addSetTickerSetDialogVisiblity"
            :visible.sync="addSetTickerSetDialogVisiblity"
            :inputData="setTickerSetDialogInput"
            :method="addSetTickerSetDialogMethod"
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

import { findTargetFromArray } from '__gUtils/busiUtils';

import baseMixin from '@/assets/mixins/baseMixin';
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';

export default {
    mixins: [ baseMixin, tickerSetMixin ],

    name: 'TickerSet',

    data () {
        this.searchFilterKey = 'name, tickers';
        return {}
    },

  
    watch: {
        tickerSets (newTickerSets) {
            this.tableList = newTickerSets
        },
    },

    methods: {
        handleSubscribeAllTickers () {
            this.subscribeAllTickers(false)
        },

        handleClickRow (row) {
            const tickerSetName = row.name;
            const targetOriginTickerSet = findTargetFromArray(this.tickerSets, 'name', tickerSetName)
            this.handleSetCurrentTickerSet(targetOriginTickerSet)
        },

        //选中行的背景颜色
        handleSelectRow (row) {
            if(row.row.name == this.currentTickerSetName) {
                return 'selected-bg'
            }
        },

        resolveTickers (tickers) {
            return tickers.map(ticker => {
                return `${ticker.instrumentId}`
            }).join(',')
        },      
    }
}
</script>

<style lang="scss">

.kf-ticker-set__warp {
    height:  100%;

    
    .ticker-set-table {
        height: 100%
    }
    
}


</style>