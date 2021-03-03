<template>
    <div class="kf-ticker-set__warp">
        <tr-dashboard title="标的池列表">
            <div slot="dashboard-header">
                <tr-dashboard-header-item>
                    <el-button size="mini" @click="handleAddTickerSet" title="添加" id="add-account-btn">添加</el-button>
                </tr-dashboard-header-item>
            </div>
            <div class="table-body ticker-set-table">
                <el-table
                    size="small"
                    :data="tableListAfterFilter"
                    height="100%"
                    @row-click="handleSetCurrentTickerSet"
                    :row-class-name="handleSelectRow"
                >
                    <el-table-column
                        prop="name"
                        label="名称"
                        show-overflow-tooltip
                    ></el-table-column>
                    <el-table-column
                        label=""
                        align="right"
                        width="60"
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



import baseMixin from '@/assets/js/mixins/baseMixin';
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';

export default {
    mixins: [ baseMixin, tickerSetMixin ],

    name: 'TickerSet',



    data () {
        this.searchFilterKey = 'name';
        return {}
    },

  
    watch: {
        tickerSets (newTickerSet) {
            this.tableList = newTickerSet
        },
    },

    methods: {
        //选中行的背景颜色
        handleSelectRow(row) {
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