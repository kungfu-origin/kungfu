<template>
  <tr-dashboard :title="todayFinish ? `当日委托 ${currentTitle}` : `未完成委托 ${currentTitle}`">
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
            v-if="[0,1,3,4,5,6,8].indexOf(+oper.status) === -1"
            class="el-icon-close mouse-over" 
            title="撤单" 
            @click="handleCancelOrder(oper)"/>
        </template>
    </tr-table>
    <date-range-dialog 
    @confirm="handleConfirmDateRange"
    :visible.sync="dateRangeDialogVisiblity"   
    :loading="exportLoading" 
    ></date-range-dialog>
  </tr-dashboard>
</template>

<script>
import { mapState } from 'vuex';

import DateRangeDialog from '../DateRangeDialog';
import tradingDataMixin from './js/tradingDataMixin';

import { dealOrder } from "__io/kungfu/watcher";
import { kungfuCancelOrder, kungfuCancelAllOrders } from '__io/kungfu/makeCancelOrder';
import { decodeKungfuLocation } from '__io/kungfu/watcher';
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
            kungfuBoardType: 'order',
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
                width: '70px'
            },{
                type: "text",
                label: "代码",
                prop: "instrumentId",
                width: '60px'
            },{
                type: "text",
                label: "",
                prop: "side",
                width: '30px'
            },{
                type: "text",
                label: "",
                prop: "offset",
                width: '30px'
            },{
                type: "number",
                label: "委托价",
                prop: "limitPrice",
                width: '80px'
            },{
                type: "text",
                align: "center",
                label: "已成交/全部",
                prop: "volumeTraded",
                width: '80px'
            },{
                type: "text",
                label: "订单状态",
                prop: "statusName",
                width: '60px'
            },{
                type: 'number',
                label: "系统延迟(μs)",
                prop: "systemLatency", 
                width: '90px'
            },{
                type: 'number',
                label: "网络延迟(μs)",
                prop: "networkLatency", 
                width: '90px'
            },{
                type: "account-strategy",
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
        kungfuData (orders) {
            const ordersResolve = this.dealOrderList(orders, {
                searchKeyword: this.searchKeyword,
                todayFinish: this.todayFinish
            });

           this.tableData = ordersResolve
        }
    },

    methods: {
        handleCancelOrder (props) {
            const kungfuLocation = decodeKungfuLocation(props.source);
            const accountId = `${kungfuLocation.group}_${kungfuLocation.name}`;
            const gatewayName = `td_${accountId}`;
            if(this.processStatus[gatewayName] !== 'online') {
                this.$message.warning(`需要先启动 TD ${accountId} 交易进程！`)
                return;
            }
            //撤单   
            if (this.moduleType === 'strategy') {
                kungfuCancelOrder( props.orderId, accountId, this.currentId)
                    .then(() => this.$message.success('撤单指令已发送！'))
                    .catch(err => this.$message.error(err.message || '撤单指令发送失败！'))
            } else if (this.moduleType === 'account') {
                kungfuCancelOrder( props.orderId, accountId)
                    .then(() => this.$message.success('撤单指令已发送！'))
                    .catch(err => this.$message.error(err.message || '撤单指令发送失败！'))
            }
        },

        handleCancelAllOrders () {

            //先判断对应进程是否启动
            if (this.moduleType === 'account') {
                if(this.processStatus[this.gatewayName] !== 'online'){
                    this.$message.warning(`需要先启动 ${this.gatewayName.toAccountId()} 交易进程！`)
                    return;
                }
            }

            this.$confirm(`确认全部撤单？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => {
                
                const orderDataList = this.tableData
                    .filter(item => {
                            return aliveOrderStatusList.includes(+item.status)
                    })

                if (this.moduleType === 'strategy') {
                    return kungfuCancelAllOrders(orderDataList, this.currentId)
                } else {
                    return kungfuCancelAllOrders(orderDataList)
                }

            })
            .then(() => this.$message.success('撤单指令已发送！'))
            .catch(err => this.$message.error(err.message || '撤单指令发送失败！'))
        },

        //查看当日已完成
        handleCheckTodayFinished () {
            this.todayFinish = true;
        },

        handleCheckTodayUnfinished () {
            this.todayFinish = false;
        },

        //对返回的数据进行处理
        dealOrderList (orders, { searchKeyword, todayFinish }) {
            let orderDataByKey = {};
            let ordersAfterFilter = orders
                .filter(item => {
                    if (searchKeyword.trim() === '') return true;
                    const { order_id, client_id, source_id, account_id, instrument_id } = item
                    const strings = [ order_id.toString(), client_id, source_id, account_id, instrument_id ].join('')
                    return strings.includes(searchKeyword) 
                });
            
            if (!todayFinish) {
                ordersAfterFilter = ordersAfterFilter
                    .filter(item => {
                        return aliveOrderStatusList.includes(+item.status)
                    })
            }

            if (this.moduleType === 'strategy') {
                ordersAfterFilter = ordersAfterFilter.filter(item => Number(item.update_time) >= this.addTime )
            }

            if (!ordersAfterFilter.length) return Object.freeze([]);

            ordersAfterFilter.kfForEach(item => {
                let orderData = dealOrder(item);
                orderData.update = true;
                orderData.systemLatency = (this.orderStat[orderData.orderId] || {}).systemLatency || '';
                orderData.networkLatency = (this.orderStat[orderData.orderId] || {}).networkLatency || '';
                orderDataByKey[orderData.id] = orderData;
            })

            return Object.freeze(Object.values(orderDataByKey).sort((a, b) =>{
                return  b.updateTimeNum - a.updateTimeNum
            }))
        }
    }
}
</script>

<style lang="scss" scoped>
    .trading-day-header {
        font-size: 10px;
        padding-left: 10px;
    }
</style>