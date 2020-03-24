
<template>
    <div class="day-chart">
        <div class="day-current pnl-statis">
            <!-- <div>累计收益率：<span :class="{'text-overflow': true, 'color-green': accumulatedPnlRatio < 0, 'color-red': accumulatedPnlRatio > 0}" :title="accumulatedPnlRatio + '%'">{{accumulatedPnlRatio + '%'}}</span> </div> -->
            <div v-if="accumulatedPnl !== ''">累计收益：<span :class="{'text-overflow': true, 'color-green': accumulatedPnl < 0, 'color-red': accumulatedPnl > 0}" :title="accumulatedPnl">{{accumulatedPnl}}</span></div>
        </div>
        <tr-no-data v-if="(dailyPnl.length == 0) || !rendererPnl" />
        <div id="daily-chart" v-else></div>
    </div>
</template>

<script>
import lineConfig from './config/lineEchart';
import moment from 'moment';
import { mapGetters, mapState } from 'vuex';
import { toDecimal, deepClone } from '__gUtils/busiUtils';
const { echarts } = require('@/assets/js/static/echarts.min.js')

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

        minPnl: {
            type: Array,
            default: () => ([])
        },

        dailyPnl: {
            type: Array,
            default: () => ([])
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

    computed: {
        ...mapState({
            tradingDay: state => state.BASE.tradingDay, //日期信息，包含交易日
        })
    },

    watch: {
        
        currentId (val) {
            this.resetData();        
        },
        
        value (val) {
            this.myChart && this.myChart.resize()
        },

        dailyPnl (dailyPnlList, oldPnlMinList) {
            const { timeList, pnlDataList } = this.dealDailyPnlList(dailyPnlList)
            if (!oldPnlMinList.length && dailyPnlList.length) {
                this.$nextTick().then(() => this.initChart(timeList, pnlDataList))
            } else if (oldPnlMinList.length && dailyPnlList.length) {
                this.updateChart(timeList, pnlDataList)
            }

            this.accumulatedPnl = pnlDataList[pnlDataList.length - 1]
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

            const { lastMinPnlUpdateTime, lastMinPnlValue } = this.getLastMinPnl();
            
            dailyPnlList
                .sort((a, b) => a.update_time - b.update_time)
                .kfForEach(pnlData => {
                    const updateTime = moment(Number(pnlData.update_time) / 1000000).format('MMDD');
                    
                    if (updateTime === lastMinPnlUpdateTime) {
                        timeList.push(lastMinPnlUpdateTime);
                        pnlDataList.push(lastMinPnlValue);
                        return;
                    }

                    timeList.push(updateTime);
                    const pnlValue = this.calcuAccumlatedPnl(pnlData)
                    pnlDataList.push(pnlValue)
                })

            return {
                timeList: Object.freeze(timeList), 
                pnlDataList: Object.freeze(pnlDataList)
            }
        },

        getLastMinPnl () {
            const lastMinPnl = this.minPnl
                .filter(pnlData => pnlData.trading_day === this.tradingDay)
                .sort((a, b) => b.update_time - a.update_time)
                [0];
            const lastMinPnlUpdateTime = moment(Number(lastMinPnl.update_time) / 1000000).format('MMDD');
            const lastMinPnlValue = this.calcuAccumlatedPnl(lastMinPnl)

            return {
                lastMinPnlUpdateTime,
                lastMinPnlValue
            }
        },

        calcuAccumlatedPnl(pnlData) {
            return toDecimal(+pnlData.unrealized_pnl + +pnlData.realized_pnl)
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