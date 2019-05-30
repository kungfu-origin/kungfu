<template>
    <tr-dashboard title="收益曲线">
    <div slot="dashboard-header" class="pnl-header">
        <tr-dashboard-header-item width="30px">
            <span :class="{'color-vi': chartType == 'day'}" @click="handleSelectChart('day')">日线</span>
        </tr-dashboard-header-item>    
        <tr-dashboard-header-item width="40px">
            <span :class="{'color-vi': chartType == 'min'}" @click="handleSelectChart('min')">分钟线</span>
        </tr-dashboard-header-item>

    </div>

    <div class="pnl-chart" ref="pnl-chart">
        <template v-if="chartType == 'day'">
            <DayChart ref='day-chart' v-bind="$attrs" v-on="$listeners"/>
        </template>
        <template v-if="chartType == 'min'">
            <MinChart ref='min-chart' v-bind="$attrs" v-on="$listeners"/>
        </template>
    </div>
    </tr-dashboard>
</template>
<script>

import DayChart from './DayChart';
import MinChart from './MinChart';

export default {
    props: {
       
    },
    inheritAttrs: false,
    data(){
        return {
            chartType: 'min',
        }
    },

    components: {
        MinChart,
        DayChart
    },

    methods: {
        handleSelectChart(type) {
            const t = this
            t.chartType = type
        },

        init(){
            const t = this;
            t.$refs['day-chart'] && t.$refs['day-chart'].resetData() && t.$refs['day-chart'].getDayData();
            t.$refs['min-chart'] && t.$refs['min-chart'].resetData() && t.$refs['min-chart'].getMinData();
        },
    }

}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.pnl-header{
    // position:absolute;
    // top: 0px;
    // height: 30px;
    // line-height: 30px;
    // padding-left: 8px;
    font-size: 11px;
    // z-index: 1;
    span{
        // margin-right: 8px;
        &:hover{
            color: $vi;
            cursor: pointer;
        }
    }
}
.pnl-chart{
    width: 100%;
    height: 100%;
    position: absolute;
    .nodata{
        z-index: 100
    }
}

.pnl-statis{
    position: absolute;
    right: 15px;
    top: 0px;
    color: #C3C8E2;
    font-size: 11px;
    height: 30px;
    line-height: 30px;
    z-index: 10;
    div{
        display: inline-block;
        margin-left: 10px;
        span{
            font-size: 12px;
            display: inline-block;
            vertical-align: bottom;
            max-width: 90px;
        }
    }
}


</style>
