
<template>
    <div class="day-chart">
        <div class="day-current pnl-statis">
            <div>累计收益率：<span :class="{'text-overflow': true, 'color-green': accumulatedPnlRatio < 0, 'color-red': accumulatedPnlRatio > 0}" :title="accumulatedPnlRatio + '%'">{{accumulatedPnlRatio + '%'}}</span> </div>
            <div>累计收益：<span :class="{'text-overflow': true, 'color-green': accumulatedPnl < 0, 'color-red': accumulatedPnl > 0}" :title="accumulatedPnl">{{accumulatedPnl}}</span></div>
        </div>
        <tr-no-data v-if="dayPnlData.length == 0" />
        <div id="day-chart" v-else></div>
    </div>
</template>

<script>
import lineConfig from './config/lineEchart'
import moment from 'moment'
import {mapGetters, mapState} from 'vuex'
const {echarts} = require('@/assets/js/static/echarts.min.js')

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
        nanomsgBackData: ''
    },

    data() {
        this.dataZoomBottomPadding = 70;
        this.bottomPadding = 70;
        this.myChart = null;
        return {
            dayData: [[],[]],
            dayPnlData: [],
            dayGroupKey: {}, //记录以时间为key的数据
            echartsSeries: {
                name: '累计收益',
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
                }
            }
        }
    },

    mounted() {
        const t = this
        if(t.currentId) t.getDayData()
        window.addEventListener("resize", () => { 
            t.myChart && t.myChart.resize()
        })
    },

    computed: {
        ...mapState({
            calendar: state => state.BASE.calendar, //日期信息，包含交易日
        }),

        accumulatedPnlRatio(){
            const t = this;
            if(!t.dayPnlData.length) return '--'
            return t.$utils.toDecimal(t.dayPnlData[t.dayPnlData.length - 1].accumulated_pnl_ratio, 4, 2)
        },

        accumulatedPnl(){
            const t = this;
            if(!t.dayPnlData.length) return '--'
            return t.$utils.toDecimal(t.dayPnlData[t.dayPnlData.length - 1].accumulated_pnl, 2)
        }
    },

    watch: {
        value(val) {
            const t = this
            t.myChart && t.myChart.resize()
        },
        currentId(val) {
            const t = this
            t.resetData();
            if(!val) return;
            t.getDayData()            
        },

        dayPnlData(newVal, oldVal){
            const t = this;
            if(oldVal.length === 0 && newVal.length !== 0){
                t.initChart()
            }
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
            if(dom){
                t.myChart = echarts.getInstanceByDom(dom)
                if( t.myChart === undefined){ 
                    t.myChart = echarts.init(dom)
                }
                let defaultConfig = t.$utils.deepClone(lineConfig)  
                defaultConfig.xAxis.data = t.dayData[0]
                //自定义标签
                defaultConfig.tooltip.formatter = function(params) {
                    if(!params && !params.length) return
                    const time = params[0].name
                    const {accumulated_pnl_ratio, accumulated_pnl, intraday_pnl_ratio, intraday_pnl, trading_day} = t.dayGroupKey[time]
                    const html = '<div style="color:#c7cce6;font-size: 11px"><div>' + trading_day + '</div>' + 
                        '<div>累计收益率：' +  t.$utils.toDecimal(accumulated_pnl_ratio, 4, 2) + '%</div>' +
                        '<div>累计盈亏：' +  t.$utils.toDecimal(accumulated_pnl, 2) + '</div>'
                    return html
                }

                defaultConfig.series = {
                    data: t.dayData[1],
                    ...t.echartsSeries
                }
                t.myChart.setOption(defaultConfig)
            }
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
                data && data.map(item => {
                    const time = item.trading_day
                    t.dayGroupKey[time] = item
                    xAxisData.push(time)
                    serirsData.push(t.$utils.toDecimal(item.accumulated_pnl_ratio, 4, 2))
                })

                t.dayData = [xAxisData, serirsData]
                t.dayPnlData = data || []
            })
            .then(() => t.minMethod(t.currentId, t.calendar.trading_day)) //查找分钟线的数据库中的数据，拿到最后一条数据放入日线最后
            .then(minData => {
                const {length} = minData
                length && t.dealMinData(minData[length-1])
            })
            .catch(err =>{
                t.$message.error(err.message || '获取失败')
            })
            .finally(() => {
                t.initChart()
            })
        },

        //处理分钟线的数据，看是直接插入日线中还是替换日线最后一条数据
        dealMinData(data) {
            const t = this
            const time = data.trading_day
            t.dayGroupKey[time] = data
            const timeLength = t.dayData[0].length
            //判断最后一条数据是否是当天的，是当天的是替换新的，不是则插入
            if(timeLength && t.dayData[0][timeLength - 1] === time) {
                t.dayData[1].pop()
                t.dayData[1].push(data.accumulated_pnl_ratio * 100) //修改y轴最后一个数据
                t.dayPnlData.pop()
                t.dayPnlData.push(Object.freeze(data))//修改保存的所有数据的最后一个数据
            }else{
                t.dayData[0].push(time)
                t.dayData[1].push(t.$utils.toDecimal(data.accumulated_pnl_ratio, 4, 2))
                t.dayPnlData.push(Object.freeze(data))
            }
        },

        dealNanomsg(nanomsg) {
            const t = this
            t.dealMinData(nanomsg)
            t.myChart && t.myChart.setOption({
                series: {
                    ...t.echartsSeries,
                    data: t.dayData[1]
                },
                xAxis: {data:t.dayData[0]}
            })
        },

        resetData(){
            const t = this;
            t.dayData = [[],[]];
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