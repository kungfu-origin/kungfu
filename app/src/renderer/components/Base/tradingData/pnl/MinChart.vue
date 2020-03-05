
<template>
    <div class="min-chart">
        <div class="min-current pnl-statis">
            <!-- <div>日内收益率：<span :class="{'text-overflow': true, 'color-green': intradayPnlRatio < 0, 'color-red': intradayPnlRatio > 0}" :title="intradayPnlRatio + '%'">{{intradayPnlRatio + '%'}}</span> </div> -->
            <div>日内盈亏：<span :class="{'text-overflow': true, 'color-green': intradayPnl < 0, 'color-red': intradayPnl > 0}" :title="intradayPnl">{{intradayPnl}}</span></div>
        </div>
        <tr-no-data v-if="kungfuData.length == 0 && rendererPnl"/>
        <div id="min-chart" v-else></div>
    </div>
</template>

<script>
import lineConfig from './config/lineEchart'
import moment from 'moment'
import { mapState } from 'vuex'
import { toDecimal, deepClone } from '__gUtils/busiUtils';
const { echarts } = require('@/assets/js/static/echarts.min.js')

export default {
    name: 'min-chart',
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

        kungfuData: {
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
            minData: [Object.freeze([]), Object.freeze([])],
            rendererPnl: false
        }
      
    },

    computed:{
        ...mapState({
            tradingDay: state => state.BASE.tradingDay, //日期信息，包含交易日
        }),

        intradayPnl(){
            const t = this;
            if(!t.kungfuData.length) return '--';
            return t.calcuIntradayPnl(t.kungfuData[t.kungfuData.length - 1])
        }
    },

    
    mounted() {
        const t = this;
        t.rendererPnl = true;
        t.resetData();
        t.initChart();
    },

    watch: {
        currentId(val) {
            this.resetData();
        },

        value(val) {
            this.myChart && this.myChart.resize()
        },

        kungfuData (pnlMinList, oldPnlMinList) {
            const { timeList, pnlDataList } = this.dealMinPnlList(pnlMinList)
            if (!oldPnlMinList.length && pnlMinList.length) {
                this.$nextTick().then(() => this.initChart(timeList, pnlDataList))
            } else if (oldPnlMinList.length && pnlMinList.length) {
                this.updateChart(timeList, pnlDataList)
            }
        },

        //检测交易日的变化，当变化的时候，重新获取数据
        tradingDay() {
            this.resetData();            
        }
    },

    methods:{
        initChart(timeList, pnlDataList) {
            const t = this
            const dom = document.getElementById('min-chart');
            if(!dom) return;
            t.myChart = echarts.getInstanceByDom(dom)
            if(t.myChart === undefined) t.myChart = echarts.init(dom);
            let defaultConfig = deepClone(lineConfig)  
            defaultConfig.xAxis.data = timeList
            defaultConfig.series = { data: pnlDataList, ...t.echartsSeries }
            t.myChart.setOption(defaultConfig)
        },

        dealMinPnlList (pnlMinList) {
            const t = this;
            let timeList = [], pnlDataList = [];

            pnlMinList
                .filter(pnlData => pnlData.trading_day === this.tradingDay)
                .kfForEach(pnlData => {
                    const updateTime = moment(Number(pnlData.update_time) / 1000000).format('HH:mm');
                    timeList.push(updateTime);
                    const pnlValue = t.calcuIntradayPnl(pnlData)
                    pnlDataList.push(pnlValue)
                })

            return {
                timeList: Object.freeze(timeList), 
                pnlDataList: Object.freeze(pnlDataList)
            }
        },

        calcuIntradayPnl(pnlData) {
            return toDecimal(pnlData.unrealized_pnl + pnlData.realized_pnl)
        },

        updateChart(timeList, pnlDataList) {
            const t = this;
            t.myChart && t.myChart.setOption({
                series: {
                    data: pnlDataList,
                    ...t.echartsSeries
                },
                xAxis: {
                    data: timeList
                }
            })
        },

        resetData(){
            const t = this;
            t.myChart && t.myChart.clear();
            t.myChart = null;
            t.minData = [ Object.freeze([]), Object.freeze([]) ];
            return true;
        },
    }
}
</script>

<style lang="scss" scoped>
@import '@/assets/scss/skin.scss';
.min-chart{
    height: 100%;
    width: 100%;
    position: relative;
    #min-chart{
        height: 100%;
        width: 100%;
    }
}
</style>
