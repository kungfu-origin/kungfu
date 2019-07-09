<template>
  <tr-dashboard :title="todayFinish ? '当日委托':'未完成委托'">
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
        <tr-dashboard-header-item v-if="!todayFinish">
            <i class="el-icon-s-claim mouse-over" title="当日委托" @click="handleCheckTodayFinished"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-else>
            <i class="el-icon-s-release mouse-over" title="未完成委托" @click="handleCheckTodayUnfinished"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-button size="mini" type="danger" style="color: #fff" title="全部撤单" @click="handleCancelAllOrders">全部撤单</el-button>
        </tr-dashboard-header-item>
    </div>
    <tr-table
    v-if="rendererTable"
    :data="tableData"
    :schema="schema"
    :renderCellClass="renderCellClass"
    >
        <template v-slot:oper="{ oper }">
            <i 
            v-if="[3,4,5,6].indexOf(+oper.status) === -1"
            class="el-icon-close mouse-over" 
            title="撤单" 
            @click="handleCancelOrder(oper)"/>
        </template>
    </tr-table>
    <date-range-dialog 
    @confirm="handleConfirmDateRange"
    :visible.sync="dateRangeDialogVisiblity"    
    ></date-range-dialog>
  </tr-dashboard>
</template>

<script>
import moment from "moment"
import { debounce, throttle, dealOrder } from "__gUtils/busiUtils";
import { writeCSV } from '__gUtils/fileUtils';
import DateRangeDialog from './DateRangeDialog';
import { nanoCancelOrder, nanoCancelAllOrder } from '__io/nano/nanoReq';
import { mapState } from 'vuex';
export default {
    name: "current-orders",
    props: {
        currentId: {
            type: String,
            default:''
        },
        moduleType: {
            type: String,
            default:''
        },
        gatewayName: {
            type: String,
            default:''
        },
        getDataMethod: {
            type: Function,
            default: () => {}
        },
        nanomsgBackData: '',
    },

    data() {
        this.orderDataByKey = {}; //为了把object 转为数据要用的list
        return {
            rendererTable: false,
            searchKeyword: "",
            filter: {
                id: '', //对id、代码id、策略id模糊查询
                dateRange: null 
            },
            getDataLock: false,
            tableData: Object.freeze([]),

            dateRangeDialogVisiblity: false,
            todayFinish: false, //为 ture 显示当日已完成
        };
    },

    components: {
        DateRangeDialog
    },

    computed: {
        ...mapState({
            accountList: state => state.ACCOUNT.accountList,
            calendar: state => state.BASE.calendar, //日期信息，包含交易日
            processStatus: state => state.BASE.processStatus
        }),

        schema(){
            return  [
            {
                type: 'text',
                label: '',
                prop: "orderId",
                width: '60px'
            },
            {
                type: "text",
                label: "下单时间",
                prop: "insertTime",
                width: '160px'
            },{
                type: "text",
                label: "代码",
                prop: "instrumentId",
                width: '70px'
            },{
                type: "text",
                label: "买卖",
                prop: "side",
            },{
                type: "text",
                label: "开平",
                prop: "offset",
            },{
                type: "text",
                label: "委托价",
                prop: "limitPrice",
            },{
                type: "text",
                label: "已成交/全部",
                prop: "volumeTraded",
            },{
                type: "text",
                label: "订单状态",
                prop: "statusName",
            },{
                type: "text",
                label: this.moduleType == 'account' ? '策略' : '账户',
                prop: this.moduleType == 'account' ? 'clientId' : 'accountId',
            },{
                type: 'operation',
                label: '',
                prop: 'oper',
                width: '40px'
            }]
        }
    },

    watch: {
        searchKeyword: debounce(function(value) {
            this.filter.id = value;
        }),

        filter: {
            deep: true,
            handler(){
                const t = this;
                !!t.currentId && t.init()
            }
        },

        currentId(val, oldVal) {
            const t = this;
            t.resetData();
            if(!val) return;
            t.rendererTable = false;
            t.$nextTick().then(() => {
                t.rendererTable = true;
                t.init()
            })
        },

        //接收推送返回的数据
        nanomsgBackData(val) {
            const t = this;
            if(!val || t.getDataLock) return
            t.dealNanomsg(val)
        }
    },

    mounted() {
        const t = this;
        t.rendererTable = true;
        t.resetData();
        t.currentId && t.init();
    },

    methods: {
        handleRefresh(){
            const t = this;
            t.currentId && t.init()
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
                    title: '委托记录',
                }).then(filename => {
                    if(!filename) return;
                    writeCSV(filename, res)
                })
            })
        },

        handleCancelOrder(props){
            const t = this;
            //防止柜台不相同，但accountId相同
            const accountIds = t.getSourceNameByAccountId(props.accountId)
            if(!accountIds.length) {
                t.$message.error(`${props.accountId} 不在系统内！`)
                return;
            }
            const accountId = accountIds[0]
            const gatewayName = `td_${accountId}`
            if(t.processStatus[gatewayName] !== 'online') {
                t.$message.warning(`需要先启动 ${accountId} 交易进程！`)
                return;
            }
            //撤单
            t.$message.info('正在发送撤单指令...')           
            nanoCancelOrder({
                gatewayName,
                orderId: props.orderId
            })
            .then(() => t.$message.success(`撤单指令已发送！`))
        },

        handleCancelAllOrders(){
            const t = this;
            //先判断对应进程是否启动
            if(t.moduleType === 'account'){
                if(t.processStatus[t.gatewayName] !== 'online') {
                    t.$message.warning(`需要先启动 ${t.gatewayName.toAccountId()} 交易进程！`)
                    return;
                }
            }else if(t.moduleType === 'strategy'){
                if(t.processStatus[t.currentId] !== 'online') {
                    t.$message.warning(`需要先启动 ${t.currentId} 策略进程！`)
                    return;
                }
            }

            t.$confirm(`确认全部撤单？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => t.$message.info('正在发送撤单指令...'))
            .then(() => nanoCancelAllOrder({
                targetId: t.moduleType === 'account' ? t.gatewayName : t.currentId,
                cancelType: t.moduleType,
                id: t.currentId
            }))
            .then(() => t.$message.success('撤单指令已发送！'))
            .catch((err) => {
                if(err == 'cancel') return
                t.$message.error(err.message || '操作失败！')
            })

        },

        //查看当日已完成
        handleCheckTodayFinished(){
            const t = this;
            t.todayFinish = true;
            const tradingDay = t.calendar.trading_day;
            const momentDay = tradingDay ? moment(tradingDay) : moment();
            //获取当天是日期范围
            const startDate = momentDay.format('YYYY-MM-DD')
            const endDate = momentDay.add(1,'d').format('YYYY-MM-DD')
            t.filter.dateRange = [startDate, endDate];
            !!t.currentId && t.init()
        },

        handleCheckTodayUnfinished(){
            const t = this;
            t.resetData();
            !!t.currentId && t.init()
        },

        init: debounce(function() {
            const t = this
            t.getData();
        }),

        getData() {
            const t = this
            if(t.getDataLock) throw new Error('get-data-lock')
            t.getDataLock = true
            //clear Data
            t.orderDataByKey = {};
            t.tableData = []
            return t.getDataMethod(t.currentId, t.filter, t.calendar.trading_day)
            .then(res => {
                if(!res) {
                    t.tableData = Object.freeze([]);                    
                    return;
                }
                const {tableData, orderDataByKey}  = t.dealData(res);
                t.tableData = Object.freeze(tableData);
                t.orderDataByKey = orderDataByKey;  
            })
            .finally(() => {
                t.getDataLock = false
            })
        },

        //对返回的数据进行处理
        dealData(data) {
            const t = this
            const preDealData = data || []
            let orderDataByKey = {};
            let tableData = []
            preDealData.map(item => {
                const dealItem = dealOrder(item)
                tableData.push(dealItem)
                orderDataByKey[item.order_id] = dealItem
            })
            return {tableData, orderDataByKey}
        },

        resetData() {
            const t = this;
            t.todayFinish = false;
            t.searchKeyword = "";
            t.filter = {
                id: '',
                dateRange: null
            };
            t.tableData = Object.freeze([]);
            t.getDataLock = false;
            return true;
        },

        //对nanomsg推送回来的数据进行处理
        dealNanomsg(data) {
            const t = this
            //当有日期筛选的时候，不需要推送数据
            if(!!t.filter.dateRange) return
            const {order_id, status, client_id} = data
            //策略页面收到的数据判断该数据是否是本策略的
            if(t.moduleType == 'strategy' && t.currentId != client_id ) return
            const {id} = t.filter
            //推送的时候也要满足筛选条件
            if(!(order_id.toString().includes(id) || data.instrument_id.includes(id) || data.client_id.includes(id))) return

            //status为3,4,5,6的时候不显示在当前委托中
            if([3, 4, 5, 6].indexOf(+status) !== -1) {
                if(!t.orderDataByKey[order_id]) return
                t.orderDataByKey[order_id] = null;
                delete t.orderDataByKey[order_id]
            }else {
                t.orderDataByKey[order_id] = dealOrder(data)
            }
            //更新数据, 根据ID来排序
            const sortOrderList = Object.values(t.orderDataByKey).sort((a, b) =>{
                return b.orderId - a.orderId
            })
            t.tableData = Object.freeze(sortOrderList.slice(0, 200))
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
            if(prop === 'statusName'){
                if(item.statusName === '错误') return 'red'
                else if(['已成交', '部分撤单', '已撤单'].indexOf(item.statusName) != -1) return 'green'
                else return 'gray'
            }
        },

        getSourceNameByAccountId(accountId){
            const t = this;
            return t.accountList.filter(a => a.account_id.indexOf(accountId) !== -1).map(a => a.account_id)
        },

        updateProcessStatus(res){
            const t = this;
            t.processStatus = res || {}
        },

    }
}
</script>

<style lang="scss">
</style>