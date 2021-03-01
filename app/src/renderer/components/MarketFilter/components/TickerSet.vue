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
                >
                    <el-table-column
                        prop="name"
                        label="名称"
                        show-overflow-tooltip
                        width="60"
                    ></el-table-column>
                    <el-table-column
                        prop="tickers"
                        label="包含标的"
                        show-overflow-tooltip
                    >
                        <template slot-scope="props">
                            <span>{{ resolveTickers(props.row.tickers) }}</span>
                        </template>
                    </el-table-column>
                    <el-table-column
                        label=""
                        align="right"
                        width="80"
                    >
                        <span class="tr-oper"><i class="el-icon-setting mouse-over" title="设置"></i></span>
                        <span class="tr-oper-delete"><i class=" el-icon-delete mouse-over" title="删除"></i></span>
                    </el-table-column>
                </el-table>
            </div>
        </tr-dashboard>

        <AddSetTickerSetDialog
            v-if="addSetTickerSetDialogVisiblity"
            :visible.sync="addSetTickerSetDialogVisiblity"
            @addTicker="handleAddTicker"
            @confirm="hanlleConfirmAddSetTickerSet"
            
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

import AddSetTickerSetDialog from './AddSetTickerSetDialog';
import AddTickerDialog from './AddTickerDialog';

import { getTickerSets, addSetTickerSet, removeTickerSetByName } from '__io/actions/market';

import baseMixin from '@/assets/js/mixins/baseMixin';

export default {
    mixins: [ baseMixin ],

    name: 'TickerSet',

    components: {
        AddSetTickerSetDialog,
        AddTickerDialog
    },

    data () {

        this.searchFilterKey = 'name';

        return {
            addSetTickerSetDialogMethod: '',
            addSetTickerSetDialogVisiblity: false,

            addTickerDialogVisiblity: false,
        }
    },

    mounted () {
        this.getTickerSet();
    },

    computed: {
        ...mapState({
            tickerSets: state => state.MARKET.tickerSets
        }),

        addSetTickerSetMethodName () {
            if (this.addSetTickerSetDialogMethod === 'add') {
                return '添加'
            } else if (this.addSetTickerSetDialogMethod === 'update') {
                return '设置'
            } else {
                return ''
            }
        },
    },

    watch: {
        tickerSets (newTickerSet) {
            this.tableList = newTickerSet
        },
    },

    methods: {
        hanlleConfirmAddSetTickerSet (tickerSet) {
            return addSetTickerSet(tickerSet)
                .then(() => {
                    this.$message.success(`${this.addSetTickerSetMethodName}成功！`)
                    this.getTickerSet()
                })
                .catch(err => {
                    this.$message.error(err.message || `${this.addSetTickerSetMethodName}失败！`)
                })
        },

        handleAddTickerConfirm (tickerData) {
            this.$bus.$emit('add-ticker-for-ticker-set', Object.freeze(tickerData))
        },

        handleAddTicker () {
            this.addTickerDialogVisiblity = true;
        },

        handleAddTickerSet () {
            this.addSetTickerSetDialogMethod = 'add';
            this.addSetTickerSetDialogVisiblity = true;
        },

        resolveTickers (tickers) {
            return tickers.map(ticker => {
                return `${ticker.instrumentId}`
            }).join(',')
        },

        getTickerSet () {
            return getTickerSets()
                .then(res => {
                    this.$store.dispatch('getTickerSets', res)
                })
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