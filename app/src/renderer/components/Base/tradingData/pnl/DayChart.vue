
<template>
    <div class="day-chart">
        <div class="day-current pnl-statis">
            <!-- <div>累计收益率：<span :class="{'text-overflow': true, 'color-green': accumulatedPnlRatio < 0, 'color-red': accumulatedPnlRatio > 0}" :title="accumulatedPnlRatio + '%'">{{accumulatedPnlRatio + '%'}}</span> </div> -->
            <div>累计收益：<span :class="{'text-overflow': true, 'color-green': accumulatedPnl < 0, 'color-red': accumulatedPnl > 0}" :title="accumulatedPnl">{{accumulatedPnl}}</span></div>
        </div>
        <tr-no-data v-if="dayPnlData.length == 0" />
        <div id="day-chart" v-else></div>
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
        dayMethod: {
            type: Function,
            default: () => {},
        },

        minMethod: {
            type: Function,
            default: () => {},
        },
        //接收推送的数据
        nanomsgBackData: {}
    },

    data() {
        this.dataZoomBottomPadding = 70;
        this.bottomPadding = 70;
        this.myChart = null;
        this.dayGroupKey = {}; //记录以时间为key的数据
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
            dayData: [Object.freeze([]), Object.freeze([])],
            dayPnlData: [],
        }
    },

    mounted() {
        const t = this
        if(t.currentId) t.getDayData();
        window.addEventListener("resize", () => { 
            t.myChart && t.myChart.resize();
        })
    },

    computed: {
        ...mapState({
            tradingDay: state => state.BASE.tradingDay, //日期信息，包含交易日
        }),

        // accumulatedPnlRatio(){
        //     const t = this;
        //     if(!t.dayPnlData.length) return '--'
        //     const len =  t.dayPnlData.length;
        //     return t.calcuAccumlatedPnlRatio(t.dayPnlData[len - 1], t.dayPnlData[0])
        // },

        accumulatedPnl(){
            const t = this;
            if(!t.dayPnlData.length) return '--'
            const len =  t.dayPnlData.length;
            return t.calcuAccumlatedPnl(t.dayPnlData[len - 1])
        }
    },

    watch: {
        value(val) {
            const t = this
            t.myChart && t.myChart.resize()
        },

        currentId(val) {
            const t = this;
            t.resetData();
            if(val) t.getDayData();
                     
        },

        dayPnlData(newVal, oldVal){
            if(oldVal.length === 0 && newVal.length !== 0) this.initChart()
        },

        //接收推送的数据
        nanomsgBackData(val) {
            if(!val) return
            this.dealNanomsg(val)
        }
    },
    methods:{
        initChart() {
            const t = this
            const dom = document.getElementById('day-chart')
            if(!dom) return;
            t.myChart = echarts.getInstanceByDom(dom)
            if( t.myChart === undefined) t.myChart = echarts.init(dom)
            let defaultConfig = deepClone(lineConfig)  
            defaultConfig.xAxis.data = t.dayData[0] || []
            defaultConfig.series = { data: t.dayData[1] || [], ...t.echartsSeries }
            t.myChart.setOption(defaultConfig)
        },

        //获取日线数据
        getDayData() {
            const t = this
            const id = t.currentId;
            t.dayMethod(t.currentId).then(data => {
                if(id != t.currentId) return
                let xAxisData = []
                let serirsData = []
                t.dayGroupKey = {}
                data && data.forEach(item => {
                    const tradingDay = item.trading_day
                    t.dayGroupKey[tradingDay] = item
                    xAxisData.push(tradingDay)
                    const fistCashData = serirsData[0] || null
                    serirsData.push(t.calcuAccumlatedPnl(item))
                })
                t.dayData = [Object.freeze(xAxisData), Object.freeze(serirsData)]
                t.dayPnlData = Object.freeze(data || [])
            })
            .then(() => t.initChart())
            .then(() => t.minMethod(t.currentId, t.tradingDay)) //查找分钟线的数据库中的数据，拿到最后一条数据放入日线最后
            .then(minData => minData.length && t.dealMinData(minData[minData.length - 1]))
            .catch(err => t.$message.error(err.message || '获取失败'))
            .finally(() => t.updateChart())
        },

        //处理分钟线的数据，看是直接插入日线中还是替换日线最后一条数据
        dealMinData(data) {
            if(!data) return;
            const t = this
            const tradingDay = data.trading_day
            t.dayGroupKey[tradingDay] = data
            const timeLength = t.dayData[0].length
            //判断最后一条数据是否是当天的，是当天的是替换新的，不是则插入
            if(timeLength && t.dayData[0][timeLength - 1] === tradingDay) {
                let tmpDayData1 = t.dayData[1].slice();
                tmpDayData1.pop()
                tmpDayData1.push(t.calcuAccumlatedPnl(data))
                t.dayData[1] = Object.freeze(tmpDayData1);

                let tmpDayPnlData = t.dayPnlData.slice();
                tmpDayPnlData.pop();
                tmpDayPnlData.push(Object.freeze(data));
                t.dayPnlData = Object.freeze(tmpDayPnlData)
            }else{
                let tmpDayData0 = t.dayData[0].slice();
                tmpDayData0.push(tradingDay);       
                let tmpDayData1 = t.dayData[1].slice()
                tmpDayData1.push(t.calcuAccumlatedPnl(data))
                t.dayData = [Object.freeze(tmpDayData0), Object.freeze(tmpDayData1)]

                let tmpDayPnlData = t.dayPnlData.slice();
                tmpDayPnlData.push(Object.freeze(data));
                t.dayPnlData = tmpDayPnlData
            }
        },

        calcuAccumlatedPnl(pnlData) {
            return toDecimal((pnlData.unrealized_pnl || 0) + (pnlData.realized_pnl || 0), 2)
        },

        // calcuAccumlatedPnlRatio(pnlData, firstPnlData) {
        //     firstPnlData = firstPnlData || {}
        //     const latestPnl = +toDecimal((pnlData.unrealized_pnl || 0) + (pnlData.realized_pnl || 0), 2)
        //     const firstPnl = +toDecimal((firstPnlData.unrealized_pnl || 0) + (firstPnlData.realized_pnl || 0), 2)
        //     console.log(firstPnlData.initial_equity , '---')
        //     return (latestPnl - firstPnl ) / firstPnl
        // },

        dealNanomsg(nanomsg) {
            const t = this
            t.dealMinData(nanomsg)
            t.updateChart();
        },

        updateChart() {
            const t = this;
            t.myChart && t.myChart.setOption({
                series: {
                    ...t.echartsSeries,
                    data: t.dayData[1]
                },
                xAxis: {data:t.dayData[0]}
            })
        },

        resetData() {
            const t = this;
            t.dayData = [Object.freeze([]), Object.freeze([])];
            t.dayPnlData = [];
            t.dayGroupKey = {};
            t.myChart && t.myChart.clear();
            t.myChart = null;
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
    #day-chart{
        height: 100%;
        width: 100%;
    }
}
</style>