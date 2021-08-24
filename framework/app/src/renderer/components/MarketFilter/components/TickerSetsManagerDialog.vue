<template>
    <el-dialog
        width="700px"
        :visible="visible" 
        title="设置标的池"  
        :close-on-click-modal="false"
        :close-on-press-escape="true"
        @close="handleClose"
    >
        <div class="kf-tickersets-manager-dialog__warp">
            <TickerSets 
            :editingTickerSet.sync="editingTickerSet"
            ></TickerSets>
            <div class="kf-tickerset-detail_warp">
                <tr-dashboard :title="`标的池：${(editingTickerSet || {}).name || '需添加标的池'}`" >
                    <div slot="dashboard-header">
                        <tr-dashboard-header-item>
                            <el-button size="mini" @click="handleAddTicker" title="添加">添加标的</el-button>
                        </tr-dashboard-header-item>
                    </div>
                    <div class="table-body ticker-set-table">
                        <el-table
                            size="small"
                            :data="tickersInEditingTickerSet"
                            height="100%"
                        >
                            <el-table-column
                                prop="instrumentId"
                                label="标的"
                                show-overflow-tooltip
                            >
                            </el-table-column>
                            <el-table-column
                                prop="exchangeId"
                                label="交易所"
                                show-overflow-tooltip
                            >
                            </el-table-column>
                            <el-table-column
                                prop="source"
                                label="柜台"
                                show-overflow-tooltip
                            >
                                <template slot-scope="props">
                                    <el-tag
                                    v-if="(tdAccountSource[props.row.source] || {}).typeName"
                                    :type="tdAccountSource[props.row.source].type" 
                                    >
                                        {{props.row.source}}
                                    </el-tag>
                                </template>
                            </el-table-column>
                            <el-table-column
                                label=""
                                align="right"
                                width="40px"
                            >
                                <template slot-scope="props">
                                    <span class="tr-oper-delete"><i class=" el-icon-delete mouse-over" title="删除" @click.stop="handleDeleteTicker(props.row, editingTickerSet)"></i></span>
                                </template>
                            </el-table-column>
                        </el-table>
                    </div>
                </tr-dashboard>
            </div>
        </div>
    </el-dialog>
</template>

<script>

import { mapState } from 'vuex';
import { findTargetFromArray } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';

import TickerSets from '@/components/MarketFilter/components/TickerSets';
import AddTickerDialog from '@/components/MarketFilter/components/AddTickerDialog';
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';


export default {

    mixins: [ tickerSetMixin ],

    components: {
        TickerSets,
        AddTickerDialog
    },

    props: {

        visible: {
            type: Boolean,
            default: false
        },

    },

    data () {

        return {
            editingTickerSet: null
        }
    },

    computed: {
        ...mapState({
            tdAccountSource: state => state.BASE.tdAccountSource || {}
        }),

        tickersInEditingTickerSet () {
            //需要处理以触发更新
            if (!this.editingTickerSet) return [];
            const editingTickerSetName = (this.editingTickerSet || {}).name || '';
            const target = findTargetFromArray(this.tickerSets, 'name', editingTickerSetName);
            if (target) {
                return (target.tickers || [])
                    .slice(0)
                    .sort((ticker1, ticker2) => {
                        return ticker1.instrumentId.localeCompare(ticker2.instrumentId)
                    });
            } else {
                return [];
            }
        },
        

    },

    methods: {

        handleAddTicker () {
            this.$emit('addTicker', this.editingTickerSet)
        },

        handleClose () {
            this.$emit('update:visible', false);
        },
    }

}
</script>

<style lang="scss">

.kf-tickersets-manager-dialog__warp {
    height: 400px;
    display: flex;

    .kf-ticker-set__warp {
        flex: 1;
    }

    .kf-tickerset-detail_warp {
        flex: 1;
    }
}

</style>