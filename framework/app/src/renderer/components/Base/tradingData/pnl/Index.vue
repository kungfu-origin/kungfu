<template>
    <tr-dashboard :title="noTitle ? '' : '收益曲线'">
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
        noTitle: {
            type: Boolean,
            default: false
        },
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

    mounted() {
        window.addEventListener("resize", () => { 
            this.$refs['day-chart'] && this.$refs['day-chart'].myChart && this.$refs['day-chart'].myChart.resize()
            this.$refs['min-chart'] && this.$refs['min-chart'].myChart && this.$refs['min-chart'].myChart.resize()
        })
    },

    methods: {
        handleSelectChart(type) {
            this.chartType = type
        }
    }
}
</script>

<style lang="scss">

@import '@/assets/scss/skin.scss';

.pnl-header{
    font-size: 11px;
    span{
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
