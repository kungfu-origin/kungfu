
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
        this.minGroupKey = {};
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
            minData: [[],[]],
            minPnlData: [],
            rendererPnl: false
        }
      
    },

    computed:{
        ...mapState({
            calendar: state => state.BASE.calendar, //日期信息，包含交易日
        }),

        intradayPnl(){
            const t = this;
            const len = t.minPnlData.length;
            return t.calcuIntradayPnl(t.minPnlData[len - 1])
        },

        intradayPnlRatio(){
            const t = this;
            if(!t.minPnlData.length) return '--'
            const firstEquityDaily = toDecimal(t.minPnlData[0].static_equity, 2)
            return toDecimal(t.intradayPnl / firstEquityDaily , 4, 2)
        },
    },

    
    mounted() {
        this.rendererPnl = true;
        if(this.currentId) this.getMinData();
    },

    watch: {
        currentId(val) {
            if(!val)return;
            this.resetData();
            this.getMinData()
        },

        value(val) {
            this.myChart && this.myChart.resize()
        },

        //接收推送的数据
        nanomsgBackData(val) {
            if(!val) return
            this.dealNanomsg(val)
        },

        minPnlData(newVal, oldVal){
            if(oldVal.length === 0 && newVal.length !== 0) this.initChart()
        },

        //检测交易日的变化，当变化的时候，重新获取数据
        'calendar.trading_day'(val, oldVal) {
            if(!oldVal && !val && !this.currentId) return;
            this.resetData();
            this.getMinData();
        }
    },

    methods:{
        initChart() {
            const t = this
            const dom = document.getElementById('min-chart')
            if(!dom) return;
            t.myChart = echarts.getInstanceByDom(dom)
            if(t.myChart === undefined) t.myChart = echarts.init(dom);
            let defaultConfig = deepClone(lineConfig)  
            defaultConfig.xAxis.data = t.minData[0]
            defaultConfig.series = { data: t.minData[1], ...t.echartsSeries }
            t.myChart.setOption(defaultConfig)
        },

        getMinData() {
            const t = this
            const id = t.currentId;
            t.getCalendar()
            .then(() => t.minMethod(t.currentId, t.calendar.trading_day))
            .then(data => {
                //当调用的传值和当前的传值不同的是，则返回
                if(id != t.currentId) return
                let xAxisData = []
                let serirsData = []
                t.minGroupKey = {}
                data && data.forEach(item => {
                    const hhmmTime = moment(item.update_time / 1000000).format('HH:mm')
                    t.minGroupKey[hhmmTime] = item
                    xAxisData.push(hhmmTime)
                    console.log(item, t.calcuIntradayPnl(item))
                    serirsData.push(t.calcuIntradayPnl(item))
                })
                t.minData = [Object.freeze(xAxisData), Object.freeze(serirsData)]
                t.minPnlData = Object.freeze(data || [])
            })
            .catch(err => t.$message.error(err.message || '获取失败'))
            .finally(() => t.initChart())
        },

        //如果有交易日，则不获取
        getCalendar() {
            const t = this;
            if(t.calendar.trading_day) return new Promise(resolve => resolve())
            else return t.$store.dispatch('getCalendar')
        },

        dealNanomsg(nanomsg) {
            const t = this;
            const oldPnlDataLen = t.minPnlData.length;
            const hhmmTime = moment(nanomsg.update_time / 1000000).format('HH:mm')
            t.minGroupKey[hhmmTime] = nanomsg
            let tmpMinData0 = t.minData[0].slice();
            tmpMinData0.push(hhmmTime)
            t.minData[0] = tmpMinData0

            let tmpMinData1 = t.minData[1].slice();
            tmpMinData1.push(t.calcuIntradayPnl(nanomsg))
            t.minData[1] = tmpMinData1

            let tmpMinPnlData = t.minPnlData.slice();
            tmpMinPnlData.push(nanomsg)
            t.minPnlData = tmpMinPnlData

            t.$nextTick().then(() => {
                if(!oldPnlDataLen) t.initChart();
                else t.updateChart();
            })
        },

        calcuIntradayPnl(pnlData) {
            return toDecimal(pnlData.dynamic_equity - pnlData.static_equity)
        },

        updateChart() {
            const t = this;
            t.myChart && t.myChart.setOption({
                series: {
                    data: t.minData[1],
                    ...t.echartsSeries
                },
                xAxis: {
                    data:t.minData[0]
                }
            })
        },

        resetData(){
            const t = this;
            t.myChart && t.myChart.clear();
            t.myChart = null;
            t.minData = [Object.freeze([]), Object.freeze([])];
            t.minPnlData = [];
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
