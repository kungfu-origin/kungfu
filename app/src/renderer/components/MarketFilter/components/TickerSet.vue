<template>
    <div class="kf-ticker-set__warp">
        <div class="kf-ticker-set-list__warp">
            <tr-dashboard title="标的池列表">
                <tr-dashboard-header-item>
                    <el-button size="mini" @click="handleAddTickerSet" title="添加" id="add-account-btn">添加</el-button>
                </tr-dashboard-header-item>
                <el-table
                    size="smnall"
                ></el-table>
            </tr-dashboard>
        </div>
        <div class="kf-ticker-set-tabs__warp"></div>
    </div>
</template>

<script>
import { mapState } from 'vuex'
export default {

    computed: {
        ...mapState({
            tickerSets: state => state.MARKET.tickerSets
        })
    },

    methods: {
        handleAddTickerSet () {
            
        }
    }
}
</script>

<style>

</style>