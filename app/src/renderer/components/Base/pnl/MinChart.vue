
<template>
    <div class="min-chart">
        <div class="min-current pnl-statis">
            <div>日内收益率：<span :class="{'text-overflow': true, 'color-green': intradayPnlRatio < 0, 'color-red': intradayPnlRatio > 0}" :title="intradayPnlRatio + '%'">{{intradayPnlRatio + '%'}}</span> </div>
            <div>日内盈亏：<span :class="{'text-overflow': true, 'color-green': intradayPnl < 0, 'color-red': intradayPnl > 0}" :title="intradayPnl">{{intradayPnl}}</span></div>
        </div>
        <tr-no-data v-if="minPnlData.length == 0 && rendererPnl"/>
        <div id="min-chart" v-else></div>
    </div>
</template>

<script>
import lineConfig from './config/lineEchart'
import moment from 'moment'
import {mapState} from 'vuex'
const {echarts} = require('@/assets/js/static/echarts.min.js')

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
        minMethod: {
            type: Function,
            default: () => {},
        },
        //接收推送的数据
        nanomsgBackData: null
    },

    data() {
        this.dataZoomBottomPadding = 70;
        this.bottomPadding = 70;
        this.minGroupKey = {}
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
        this.myChart = null;
        return {
            minData: [[],[]],
            minPnlData: [],
            rendererPnl: false
        }
      
    },

    computed:{
        ...mapState({
            calendar: state => state.BASE.calendar, //日期信息，包含交易日
        }),

        intradayPnlRatio(){
            const t = this;
            if(!t.minPnlData.length) return '--'
            return t.$utils.toDecimal(t.minPnlData[t.minPnlData.length - 1].intraday_pnl_ratio, 4, 2)
        },

        intradayPnl(){
            const t = this;
            if(!t.minPnlData.length) return '--'
            return t.$utils.toDecimal(t.minPnlData[t.minPnlData.length - 1].intraday_pnl, 2)
        }
    },

    
    mounted() {
        const t = this
        t.rendererPnl = true;
        if(t.currentId) t.getMinData();
        window.addEventListener("resize", () => { 
            t.myChart && t.myChart.resize()
        })
    },

    watch: {
        currentId(val) {
            const t = this
            t.resetData();
            if(!val)return;
            t.getMinData()
        },

        value(val) {
            const t = this
            t.myChart && t.myChart.resize()
        },

        //接收推送的数据
        nanomsgBackData(val) {
            if(!val) return
            this.dealNanomsg(val)
        },

        minPnlData(newVal, oldVal){
            const t = this;
            if(oldVal.length === 0 && newVal.length !== 0){
                t.initChart()
            }
        },

        //检测交易日的变化，当变化的时候，重新获取数据
        'calendar.trading_day'(val, oldVal) {
            const t = this;
            if(!oldVal && !val && !t.currentId) return;
            t.resetData();
            t.getMinData();
        }
    },

    methods:{
        initChart() {
            const t = this
            const dom = document.getElementById('min-chart')
            if(dom){
                t.myChart = echarts.getInstanceByDom(dom)
                if( t.myChart === undefined){ 
                    t.myChart = echarts.init(dom)
                }
                let defaultConfig = t.$utils.deepClone(lineConfig)  
                defaultConfig.xAxis.data = t.minData[0]
                //自定义标签数据
                defaultConfig.tooltip.formatter = function(params) {
                    if(!params && !params.length) return
                    const time = params[0].name
                    const {accumulated_pnl_ratio, accumulated_pnl, intraday_pnl_ratio, intraday_pnl, update_time} = t.minGroupKey[time]
                    const html = '<div style="color:#c7cce6;font-size: 11px"><div>' + moment(update_time/1000000).format('YYYY.MM.DD HH:mm') + '</div>' + 
                        '<div>日内收益率：' +  t.$utils.toDecimal(intraday_pnl_ratio, 4, 2) + '%</div>' +
                        '<div>日内盈亏：' +  t.$utils.toDecimal(intraday_pnl, 2) + '</div> </div>' 
                    return html
                }

                defaultConfig.series = {
                    data: t.minData[1],
                    ...t.echartsSeries
                }
                t.myChart.setOption(defaultConfig) 
            }
        },

        getMinData() {
            const t = this
            const id = t.currentId;
            (() => {
                //如果有交易日，则不获取
                if(t.calendar.trading_day) return new Promise(resolve => resolve())
                else return t.$store.dispatch('getCalendar')
            })()
            .then(() => t.minMethod(t.currentId, t.calendar.trading_day))
            .then(data => {
                //当调用的传值和当前的传值不同的是，则返回
                if(id != t.currentId) {
                    return
                }
                let xAxisData = []
                let serirsData = []
                t.minGroupKey = {}
                data && data.map(item => {
                    const time = moment(item.update_time/1000000).format('HH:mm')
                    t.minGroupKey[time] = item
                    xAxisData.push(time)
                    serirsData.push(t.$utils.toDecimal(item.accumulated_pnl_ratio, 4, 2))
                })
                t.minData = [xAxisData, serirsData]
                t.minPnlData = data || []
            }).catch(err =>{
                t.$message.error(err.message || '获取失败')
            }).finally(() => {
                t.initChart()
            })
            
        },

        dealNanomsg(nanomsg) {
            const t = this;
            const oldPnlDataLen = t.minPnlData.length;
            const time = moment(nanomsg.update_time/1000000).format('HH:mm')
            t.minGroupKey[time] = nanomsg
            t.minData[0].push(time)
            t.minData[1].push(t.$utils.toDecimal(nanomsg.accumulated_pnl_ratio, 4, 2))
            t.minPnlData.push(nanomsg)
            if(!oldPnlDataLen) {t.$nextTick().then(() => t.initChart())}
            else{
                t.myChart && t.myChart.setOption({
                    series: {
                        data: t.minData[1],
                        ...t.echartsSeries
                    },
                    xAxis: {data:t.minData[0]}
                })
            }
        },

        resetData(){
            const t = this;
            this.myChart && this.myChart.clear();
            this.myChart = null;
            this.minData = [[],[]];
            this.minPnlData = [];
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
