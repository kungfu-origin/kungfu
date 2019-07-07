<template>
<tr-dashboard :title="filter.dateRange ? '历史成交' : '当日成交'">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-refresh mouse-over" title="刷新" @click="handleRefresh"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-download mouse-over" title="导出" @click="dateRangeDialogVisiblity = true"></i>
        </tr-dashboard-header-item>
    </div>
    <tr-table
        v-if="rendererTable"
        :data="tableData"
        :schema="schema"
        :renderCellClass="renderCellClass"
    ></tr-table>
    <date-range-dialog 
    @confirm="handleConfirmDateRange"
    :visible.sync="dateRangeDialogVisiblity"    
    ></date-range-dialog>
</tr-dashboard>

</template>

<script>
import { mapGetters, mapState } from 'vuex'
import moment from 'moment'
import { offsetName, sideName } from '@/assets/config/tradingConfig'
import { debounce, throttleInsert, throttle } from "@/assets/js/utils"
import { writeCSV } from '__gUtils/fileUtils';
import DateRangeDialog from './DateRangeDialog';

export default {
    name: 'trades-record',
    props: {
        currentId: {
            type: String,
            default:''
        },
        moduleType: {
            type: String,
            default:''
        },
        getDataMethod: {
            type: Function,
            default: () => {}
        },
        //接收推送的数据
        nanomsgBackData: {
            type: Object,
            default: {}
        }
    },

    data() {
        this.throttleInsertTrade = throttleInsert(500, 'unshift')
        return {
            rendererTable: false,
            searchKeyword: '',
            filter: {
                id: '',
                dateRange: null
            },
            getDataLock: false,
            tableData: Object.freeze([]),
            dateRangeDialogVisiblity: false
        }
    },

    components: {
        DateRangeDialog
    },

    computed:{
        ...mapState({
            calendar: state => state.BASE.calendar, //日期信息，包含交易日
        }),

        schema(){
            return [{
                type: 'text',
                label: '成交时间',
                prop: 'tradeTime',
                width: '160px'
            },{
                type: 'text',
                label: '代码',
                prop: 'instrumentId',
                width: '70px'
            },{
                type: 'text',
                label: '买卖',
                prop: 'side',
            },{
                type: 'text',
                label: '开平',
                prop: 'offset',
            },{
                type: 'text',
                label: '成交价',
                prop: 'price',
            },{
                type: 'text',
                label: '成交量',
                prop: 'volume',
            },{
                type: 'text',
                label: this.moduleType == 'account' ? '策略': '账户',
                prop: this.moduleType == 'account' ? 'clientId': 'accountId',
            }]
        }
    },

    watch: {
        searchKeyword: debounce(function(val) {
            const t = this;
            t.filter.id = val;
        }),

        filter:{
            deep: true,
            handler() {
                const t = this;
                t.currentId && t.init(true)
            }
        },
        
        currentId(val) {
            const t = this;
            t.resetData();
            if(!val) return;
            t.rendererTable = false;
            t.$nextTick().then(() => {
                t.rendererTable = true;
                t.init();
            })
        },

        //接收推送返回的数据
        nanomsgBackData(val) {
            const t = this;
            if(!val || t.getDataLock) return
            t.dealNanomsg(val)
        }

    },

    mounted(){
        const t = this;
        t.rendererTable = true;
        t.resetData();
        t.currentId && t.init();
    },

    methods:{
        handleRefresh(){
            const t = this;
            t.resetData();
            t.currentId && t.init();
        },

        //选择日期以及保存
        handleConfirmDateRange(dateRange){
            const t = this;
            t.getDataMethod(t.currentId, {
                id: t.filter.id,
                dateRange
            }, t.calendar.trading_day).then(res => {
                if(!res) return;
                t.$saveFile({
                    title: '成交记录',
                }).then(filename => {
                    if(!filename) return;
                    writeCSV(filename, res)
                })
            })
        },

        //重置数据
        resetData() {
            const t = this;
            this.searchKeyword = '';
            this.filter = {
                id: '',
                dateRange: null
            };
            t.getDataLock = false;
            t.tableData = Object.freeze([]);
            return true;
        },

        init: debounce(function() {
            const t = this
            t.getData()
        }),

        //首次获取数据
        getData() {
            const t = this
            if(t.getDataLock) throw new Error('get-data-lock');
            //获得获取数据的方法名
            t.getDataLock = true
            t.tableData = Object.freeze([])
            //id:用户或者交易id，filter：需要筛选的数据
            return t.getDataMethod(t.currentId, t.filter, t.calendar.trading_day).then(res => {
                if(!res || !res.length) {
                    t.tableData = Object.freeze([])
                    return;
                }
                t.tableData = Object.freeze(t.dealData(res))
            }).finally(() => t.getDataLock = false)
        },

        //对返回的数据进行处理
        dealData(data) {
            const t = this
            const historyData = data || []
            const tableData = historyData.map(item => (t.dealTrade(item)))
            return tableData
        },

        //处理需要的数据及顺序
        dealTrade(item) {
            return {
                id: item.account_id.toString() + '_' + item.trade_id.toString() + '_' + item.trade_time.toString(),
                tradeTime: item.trade_time && moment(item.trade_time/1000000).format('YYYY-MM-DD HH:mm:ss'),
                instrumentId: item.instrument_id,
                side: sideName[item.side],
                offset: offsetName[item.offset],
                price: item.price,
                volume: item.volume,
                clientId: item.client_id,
                accountId: item.account_id
            }     
        },

        dealNanomsg(data) {
            const t = this
            //当有日期筛选的时候，不需要推送数据
            if(t.filter.dateRange) return
            //如果存在筛选，则推送的数据也需要满足筛选条件
            const { id, dateRange } = t.filter
            const { trade_time } = data
            if(!((data.instrument_id.includes(id) || data.client_id.includes(id)) )) return
            const tradeData = t.dealTrade(data)
            t.throttleInsertTrade(tradeData).then(tradeList => {
                if(!tradeList) return;
                let oldTableData = t.tableData.slice(0, 500);
                oldTableData = [...tradeList, ...oldTableData]
                //更新数据
                t.tableData = Object.freeze(oldTableData)
            })
           
        },

        renderCellClass(prop, item){
            if(prop === 'side'){
                if(item.side === '买') return 'red'
                else if(item.side === '卖') return 'green'
            }
            if(prop === 'offset'){
                if(item.offset === '开仓') return 'red'
                else if(item.offset === '平仓') return 'green'
            }
        }
    }
}
</script>

<style lang="scss">
.trades-record{
    height: 100%;
}
</style>