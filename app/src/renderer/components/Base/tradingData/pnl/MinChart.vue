
<template>
    <div class="min-chart">
        <div class="min-current pnl-statis">
            <!-- <div>日内收益率：<span :class="{'text-overflow': true, 'color-green': intradayPnlRatio < 0, 'color-red': intradayPnlRatio > 0}" :title="intradayPnlRatio + '%'">{{intradayPnlRatio + '%'}}</span> </div> -->
            <div v-if="intradayPnl !== ''">日内盈亏：<span :class="{'text-overflow': true, 'color-green': intradayPnl < 0, 'color-red': intradayPnl > 0}" :title="intradayPnl">{{intradayPnl}}</span></div>
        </div>
        <tr-no-data v-if="(intradayPnl === '') || (minPnl.length == 0) || !rendererPnl"/>
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

        minPnl: {
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
            intradayPnl: ''
        }
      
    },

    computed:{
        ...mapState({
            tradingDay: state => state.BASE.tradingDay, //日期信息，包含交易日
        })
    },

    
    mounted() {
        this.rendererPnl = true;
        this.resetData();
        this.$nextTick().then(() => this.initChart())
    },

    watch: {
        currentId(val) {
            this.resetData();
        },

        value(val) {
            this.myChart && this.myChart.resize()
        },

        minPnl (minPnlList, oldPnlMinList) {
            const { timeList, pnlDataList } = this.dealMinPnlList(minPnlList)
            if ((!oldPnlMinList.length && minPnlList.length) || !this.myChart) {
                this.$nextTick().then(() => this.initChart(timeList, pnlDataList))
            } else if (oldPnlMinList.length && minPnlList.length) {
                this.updateChart(timeList, pnlDataList)
            }

            pnlDataList.length && (this.intradayPnl = pnlDataList[pnlDataList.length - 1]);

        },

        //检测交易日的变化，当变化的时候，重新获取数据
        tradingDay() {
            this.resetData();     
            this.initChart();       
        }
    },

    methods:{

        initChart (timeList=[], pnlDataList=[]) {
            const dom = document.getElementById('min-chart');
            if(!dom) return;
            this.myChart = echarts.getInstanceByDom(dom)
            if(this.myChart === undefined) this.myChart = echarts.init(dom);
            let defaultConfig = deepClone(lineConfig)  
            defaultConfig.xAxis.data = timeList
            defaultConfig.series = { data: pnlDataList, ...this.echartsSeries }
            this.myChart.setOption(defaultConfig)
        },

        dealMinPnlList (minPnlList) {
            let timeList = [], pnlDataList = [];

            minPnlList
                .filter(pnlData => pnlData.trading_day === this.tradingDay)
                .sort((a, b) => a.update_time - b.update_time)
                .kfForEach(pnlData => {
                    const updateTime = moment(Number(pnlData.update_time) / 1000000).format('HH:mm');
                    timeList.push(updateTime);
                    const pnlValue = this.calcuIntradayPnl(pnlData)
                    pnlDataList.push(pnlValue)
                })

            return {
                timeList: Object.freeze(timeList), 
                pnlDataList: Object.freeze(pnlDataList)
            }
        },

        calcuIntradayPnl (pnlData) {
            return toDecimal(+pnlData.unrealized_pnl + +pnlData.realized_pnl)
        },

        updateChart (timeList, pnlDataList) {
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

        resetData(){
            this.myChart && this.myChart.clear();
            this.myChart = null;
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
