<template>
  <tr-dashboard :title="todayFinish ? `当日委托 ${currentTitle}` : `当前委托 ${currentTitle}`">
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
            <i class="el-icon-s-release mouse-over" title="当前委托" @click="handleCheckTodayUnfinished"></i>
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
            v-if="[0,3,4,5,6].indexOf(+oper.status) === -1"
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
import { mapState } from 'vuex';
import moment from "moment"

import DateRangeDialog from '../DateRangeDialog';
import tradingDataMixin from './js/tradingDataMixin';

import { dealOrder } from "__gUtils/kungfuUtils";
import { kungfuCancelOrder } from '__io/kungfu/makeCancelOrder';
import { decodeKungfuLocation } from '__gUtils/kungfuUtils';
import { nanoCancelAllOrder } from '__io/nano/nanoReq';
import { aliveOrderStatusList } from '__gConfig/tradingConfig';
import { writeCSV } from '__gUtils/fileUtils';


export default {
    name: "current-orders",
   
   mixins: [ tradingDataMixin ],

    props: {
        gatewayName: {
            type: String,
            default:''
        },

        orderStat: {
            type: Object,
            default: () => ({})
        }
    },

    data () {
        return {
            todayFinish: true, //为 ture 显示当日已完成
        };
    },

    components: {
        DateRangeDialog
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus
        }),

        schema () {
            return  [
            {
                type: "text",
                label: "下单时间",
                prop: "updateTime",
                width: '160px'
            },{
                type: "text",
                label: "代码",
                prop: "instrumentId",
                width: '80px'
            },{
                type: "text",
                label: "买卖",
                prop: "side",
                width: '50px'
            },{
                type: "text",
                label: "开平",
                prop: "offset",
                width: '50px'
            },{
                type: "number",
                label: "委托价",
                prop: "limitPrice",
            },{
                type: "text",
                align: "center",
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
                type: 'number',
                label: "系统延迟(μs)",
                prop: "systemLatency",   
            },{
                type: 'operation',
                label: '',
                prop: 'oper',
                width: '40px'
            }]
        }
    },

    watch: {
        kungfuData (orders) {
            const t = this;
            const ordersResolve = t.dealOrderList(orders, {
                tradingDay: t.tradingDay,
                searchKeyword: t.searchKeyword,
                todayFinish: t.todayFinish
            });

            ordersResolve.length && (t.tableData = ordersResolve)
        }
    },

    methods: {
        handleCancelOrder (props) {
            const t = this;
            const kungfuLocation = decodeKungfuLocation(props.source);
            const accountId = `${kungfuLocation.group}_${kungfuLocation.name}`;
            const gatewayName = `td_${accountId}`;
            if(t.processStatus[gatewayName] !== 'online') {
                t.$message.warning(`需要先启动 TD ${accountId} 交易进程！`)
                return;
            }
            //撤单   
            if (t.moduleType === 'strategy') {
                kungfuCancelOrder( props.orderId, accountId, t.currentId)
                    .then(() => t.$message.success('撤单指令已发送！'))
                    .catch(err => t.$message.error(err.message || '撤单指令发送失败！'))
            } else if (t.moduleType === 'account') {
                kungfuCancelOrder( props.orderId, accountId)
                    .then(() => t.$message.success('撤单指令已发送！'))
                    .catch(err => t.$message.error(err.message || '撤单指令发送失败！'))
            }
           
        },

        handleCancelAllOrders () {
            const t = this;

            //先判断对应进程是否启动
            if (!t.gatewayName.toAccountId()) {
                return;
            }

            if((t.moduleType === 'account') && (t.processStatus[t.gatewayName] !== 'online')){
                t.$message.warning(`需要先启动 ${t.gatewayName.toAccountId()} 交易进程！`)
                return;
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
        handleCheckTodayFinished () {
            this.todayFinish = true;
        },

        handleCheckTodayUnfinished () {
            this.todayFinish = false;
        },

        //对返回的数据进行处理
        dealOrderList (orders, { tradingDay, searchKeyword, todayFinish }) {
            const t = this
            let orderDataByKey = {};

            let ordersAfterFilter = orders
            .filter(item => (item.trading_day === tradingDay))
            .filter(item => {
                if (searchKeyword.trim() === '') return true;
                const { order_id, client_id, source_id, account_id, instrument_id } = item
                const strings = [ order_id.toString(), client_id, source_id, account_id, instrument_id ].join('')
                return strings.includes(searchKeyword) 
            })
            .filter(item => (todayFinish ? true : aliveOrderStatusList.includes(+item.status)));

            if (t.moduleType === 'strategy') {
                ordersAfterFilter = ordersAfterFilter.filter(item => Number(item.update_time) >= t.addTime )
            }

            if (!ordersAfterFilter.length) return Object.freeze([]);

            ordersAfterFilter.kfForEach(item => {
                let orderData = dealOrder(item);
                orderData.update = true;
                orderData.systemLatency = (t.orderStat[orderData.orderId] || {}).systemLatency || '';
                orderDataByKey[orderData.id] = orderData;
            })

            return Object.freeze(Object.values(orderDataByKey).sort((a, b) =>{
                return  b.updateTimeNum - a.updateTimeNum
            }))
        }
    }
}
</script>

<style lang="scss">
</style>