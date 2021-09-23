
<template>
    <div class="day-chart">
        <div class="day-current pnl-statis">
            <!-- <div>累计收益率：<span :class="{'text-overflow': true, 'color-green': accumulatedPnlRatio < 0, 'color-red': accumulatedPnlRatio > 0}" :title="accumulatedPnlRatio + '%'">{{accumulatedPnlRatio + '%'}}</span> </div> -->
            <div v-if="accumulatedPnl !== ''">累计收益：<span :class="{'text-overflow': true, 'color-green': accumulatedPnl < 0, 'color-red': accumulatedPnl > 0}" :title="accumulatedPnl">{{ accumulatedPnl }}</span></div>
        </div>
        <tr-no-data v-if="(accumulatedPnl === '') || (dailyPnl.length == 0) || !rendererPnl" />
        <div id="daily-chart" v-else></div>
    </div>
</template>

<script>
import lineConfig from './config/lineEchart';
import { toDecimal, deepClone } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';
import echarts from '@/assets/js/static/echarts.min.js'

export default {
    name: 'day-chart',

    props: {
        value: false,
        currentId: {
            type: String,
            default: '',
        },

        moduleType: {
            type: String,
            default: "",
        },

        dailyPnl: {
            type: Array,
            default: () => ([])
        },

        addTime: {
            type: Number,
            default: 0
        }
    },

    data() {
        this.myChart = null;
        this.echartsSeries = {
            type: 'line',
            showSymbol: false, //默认不显示原点，鼠标放上会显示
            areaStyle: {
                normal: {
                    color: new echarts.graphic.LinearGradient(0, 0, 0, 1, [{
                        offset: 0,
                        color: 'rgba(30, 90, 130, 1)'
                    }, {
                        offset: 1,
                        color: 'rgba(22, 27, 46, 0)'
                    }])
                }
            },
        };
        return {
            rendererPnl: false,
            accumulatedPnl: ''
        }
    },

    mounted () {
        this.rendererPnl = true;
        this.resetData();
        this.$nextTick().then(() => this.initChart())
    },

    watch: {
        
        currentId (val) {
            this.resetData();        
        },
        
        value (val) {
            this.myChart && this.myChart.resize()
        },

        dailyPnl (dailyPnlList, oldPnlMinList) {
            const { timeList, pnlDataList } = this.dealDailyPnlList(dailyPnlList.slice(0))

            if ((!oldPnlMinList.length && dailyPnlList.length) || !this.myChart) {
                this.$nextTick().then(() => this.initChart(timeList, pnlDataList))
            } else if (oldPnlMinList.length && dailyPnlList.length) {
                this.updateChart(timeList, pnlDataList)
            }

            pnlDataList.length && (this.accumulatedPnl = pnlDataList[pnlDataList.length - 1])
        }
    },
    methods:{

        initChart(timeList=[], pnlDataList=[]) {
            const dom = document.getElementById('daily-chart');
            if(!dom) return;
            this.myChart = echarts.getInstanceByDom(dom)
            if(this.myChart === undefined) this.myChart = echarts.init(dom);
            let defaultConfig = deepClone(lineConfig)  
            defaultConfig.xAxis.data = timeList
            defaultConfig.series = { data: pnlDataList, ...this.echartsSeries }
            this.myChart.setOption(defaultConfig)
        },


        dealDailyPnlList (dailyPnlList) {
            let timeList = [], pnlDataList = [];
            dailyPnlList
                .filter(pnlData => Number(pnlData.updateTimeNum) >= this.addTime)
                .reverse()
                .kfForEach(pnlData => {
                    const tradingDay = pnlData.tradingDay.slice(4)
                    timeList.push(tradingDay);
                    const pnlValue = this.calcuAccumlatedPnl(pnlData)
                    pnlDataList.push(pnlValue)
                })

            return {
                timeList: Object.freeze(timeList), 
                pnlDataList: Object.freeze(pnlDataList)
            }
        },

        calcuAccumlatedPnl(pnlData) {
            return toDecimal(+pnlData.unrealizedPnl + +pnlData.realizedPnl)
        },

        updateChart(timeList, pnlDataList) {
            this.myChart && this.myChart.setOption({
                series: {
                    data: pnlDataList,
                    ...this.echartsSeries
                },
                xAxis: {
                    data: timeList
                }
            })
        },

        resetData() {
            this.myChart && this.myChart.clear();
            this.myChart = null;
            return true;
        }
    }
}
</script>

<style lang="scss" scoped>
@import '@/assets/scss/skin.scss';
.day-chart{
    height: 100%;
    width: 100%;
    position: relative;

    #daily-chart{
        height: 100%;
        width: 100%;
    }
}
</style>