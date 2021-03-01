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
                        prop="tickerSetName"
                        label="名称"
                        show-overflow-tooltip
                        min-width="80"
                    ></el-table-column>
                    <el-table-column
                        prop="tickers"
                        label="包含标的"
                    ></el-table-column>
                    <el-table-column
                    label=""
                    align="right"
                    min-width="120"
                    >
                    </el-table-column>
                </el-table>
            </div>
        </tr-dashboard>

        <AddSetTickerSetDialog
            v-if="addSetTickerSetDialogVisiblity"
            :visible.sync="addSetTickerSetDialogVisiblity"
        ></AddSetTickerSetDialog>
    </div>
</template>

<script>
import { mapState } from 'vuex';

import AddSetTickerSetDialog from './AddSetTIckerSetDialog';

import baseMixin from '@/assets/js/mixins/baseMixin';

export default {
    mixins: [ baseMixin ],

    name: 'TickerSet',

    components: {
        AddSetTickerSetDialog
    },

    data () {

        return {
            addSetTickerSetDialogMethod: '',
            addSetTickerSetDialogVisiblity: false
        }
    },

    computed: {
        ...mapState({
            tickerSets: state => state.MARKET.tickerSets
        })
    },

    methods: {
        handleAddTickerSet () {
            this.addSetTickerSetDialogMethod = 'add';
            this.addSetTickerSetDialogVisiblity = true;
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