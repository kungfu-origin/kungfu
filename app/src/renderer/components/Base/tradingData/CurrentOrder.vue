<template>
  <tr-dashboard :title="title">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest && !dateForHistory">
            <i class="el-icon-date mouse-over" title="历史" @click="dateRangeDialogVisiblityForHistory = true"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest && dateForHistory">
            <span>{{ dateForHistory }}</span>
            <i class="el-icon-close mouse-over" @click="handleClearHistory"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!todayFinish && !ifBacktest">
            <i class="el-icon-s-claim mouse-over" title="委托记录" @click="handleCheckTodayFinished"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-else-if="!ifBacktest">
            <i class="el-icon-s-release mouse-over" title="未完成委托" @click="handleCheckTodayUnfinished"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest">
            <i class="el-icon-download mouse-over" title="导出" @click="dateRangeDialogVisiblityForExport = true"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!value">
            <i class="el-icon-monitor mouse-over" title="打开监控" @click="handleMonitOrders"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-else>
            <i class="el-icon-s-platform mouse-over" title="关闭监控" @click="handleMonitOrders"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!ifBacktest">
            <el-button size="mini" type="danger" style="color: #fff" title="全部撤单" @click="handleCancelAllOrders">全部撤单</el-button>
        </tr-dashboard-header-item>
    </div>
    <!-- handleShowDetail -->
    <tr-table
    v-if="rendererTable"
    :data="tableData"
    :schema="schema"
    :renderCellClass="renderCellClass"
    @dbclickRow="handleCancelOrder"
    @clickCell="handleShowAdjustOrder"
    @rightClickRow="handleShowDetail"
    >
    </tr-table>
    <date-picker-dialog 
    @confirm="handleConfirmDateRangeForExport"
    v-if="dateRangeDialogVisiblityForExport"
    :visible.sync="dateRangeDialogVisiblityForExport"   
    :loading="dateRangeExportLoading" 
    ></date-picker-dialog>
     <date-picker-dialog 
    @confirm="handleConfirmDateRangeForHistory"
    v-if="dateRangeDialogVisiblityForHistory"
    :visible.sync="dateRangeDialogVisiblityForHistory"   
    :loading="dateRangeExportLoading" 
    ></date-picker-dialog>

    <div class="kf-ajust-order-in-orders-dashboard__warp">
        <div class="mask" v-if="adjustOrderInputVisibility" @click="handleHideAdjustOrder"></div>
        <div 
        class="kf-ajust-order-in-orders-dashboard__content" 
        v-if="adjustOrderInputVisibility"
        :style="{
            left: adjustOrderInputSizeData.left,
            top: adjustOrderInputSizeData.top,
            width: adjustOrderInputSizeData.width,
            height: adjustOrderInputSizeData.height
        }"
        >
        <el-input-number
            v-if="adjustOrderProp === 'limitPrice'"
            :precision="3"
            :step="0.001"
            :controls="false"
            placeholder="价格"
            v-model.trim="adjustOrderForm.limit_price"
            @blur="handleBlurAdjustOrderInput('price')"
            >
            </el-input-number>                
        <el-input-number
            v-if="adjustOrderProp === 'volumeTraded'"
            :step="100"  
            :controls="false"
            placeholder="数量"
            v-model.trim="adjustOrderForm.volume"
            @blur="handleBlurAdjustOrderInput('volume')"                                    
            ></el-input-number>   
        </div>
    </div>

  </tr-dashboard>
</template>

<script>
import { mapState } from 'vuex';

import DatePickerDialog from '../DatePickerDialog';
import tradingDataMixin from './js/tradingDataMixin';

import { dealOrder } from "__io/kungfu/watcher";
import { kungfuCancelAllOrders } from '__io/kungfu/makeCancelOrder';
import { aliveOrderStatusList } from '__gConfig/tradingConfig';
import { writeCSV } from '__gUtils/fileUtils';

import makeOrderMixin from '@/components/Base/tradingData/js/makeOrderMixin';
import makeOrderCoreMixin from '@/components/Base/tradingData/js/makeOrderCoreMixin';

export default {
    name: "current-orders",
   
   mixins: [ tradingDataMixin, makeOrderMixin, makeOrderCoreMixin ],

    props: {
        gatewayName: {
            type: String,
            default:''
        },

        orderStat: {
            type: Object,
            default: () => ({})
        },

        name: {
            type: String,
            default: ''
        }
    },

    data () {
        return {
            kungfuBoardType: 'order',
            todayFinish: true, //为 ture 显示当日已完成
        };
    },

    components: {
        DatePickerDialog
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus
        }),

        gatewayNameResolved () {
            if (this.gatewayName) return `td_${this.gatewayName}`;
            else return ''
        },

        title () {
            if (this.name) return this.name;
            return this.todayFinish ? `委托记录 ${this.currentTitle}` : `未完成委托 ${this.currentTitle}`
        },

        schema () {
            if (this.dateForHistory) {
                return [
                {
                    type: "text",
                    label: "下单时间",
                    prop: "updateTimeMMDD",
                    width: "140px"
                },{
                    type: "text",
                    label: "代码",
                    prop: "instrumentId",
                    width: '60px'
                },{
                    type: "text",
                    label: "",
                    prop: "side",
                    width: '40px'
                },
                {
                    type: "text",
                    label: "",
                    prop: "offset",
                    width: '40px'
                },
                {
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
                width: '40px',
            },
            {
                type: "text",
                label: "",
                prop: "offset",
                width: '40px'
            },
            {
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
                prop: "latencySystem", 
                width: '90px'
            },{
                type: 'number',
                label: "网络延迟(μs)",
                prop: "latencyNetwork", 
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
            .filter(item => !!item)
        }
    },

    watch: {
        kungfuData (orders) {
            const ordersResolve = this.dealOrderList(orders, {
                searchKeyword: this.searchKeyword,
                todayFinish: this.todayFinish
            });

           this.tableData = ordersResolve;
        }
    },

    methods: {
        handleShowDetail (row) {
            let orderData = JSON.parse(JSON.stringify(row));
            let orderMessage = '';

            delete orderData.id;
            delete orderData.source;
            delete orderData.dest;
            delete orderData.updateTimeNum;
            delete orderData.updateTime;
            delete orderData.update;
            delete orderData.sourceId;
            delete orderData.status;

            orderData.accountId = orderData.accountId.split('手动').join(' ')
            
            Object.keys(orderData || {}).forEach(key => {
                const value = orderData[key];
                orderMessage += `${key}: ${value} </br>`
            })

            this.$alert(orderMessage, `委托详情 ${orderData.orderId}`, {
                confirmButtonText: '确定',
                dangerouslyUseHTMLString: true,
                closeOnPressEscape: true,
                callback: () => {}
            });
        },

        handleCancelAllOrders () {

            //先判断对应进程是否启动
            if (this.moduleType === 'account') {

                if (!this.gatewayNameResolved) {
                    this.$message.warning(`需要先添加交易进程！`)
                    return   
                }

                if(this.processStatus[this.gatewayNameResolved] !== 'online'){
                    this.$message.warning(`需要先启动 ${this.gatewayNameResolved.toAccountId()} 交易进程！`)
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
            .catch(err => {
                if(err == 'cancel') return;
                this.$message.error(err.message || '撤单指令发送失败！')
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
                orderData.latencySystem = (this.orderStat[orderData.orderId] || {}).latencySystem || '';
                orderData.latencyNetwork = (this.orderStat[orderData.orderId] || {}).latencyNetwork || '';
                orderDataByKey[orderData.id] = Object.freeze(orderData);
            })

            return Object.freeze(Object.values(orderDataByKey).sort((a, b) =>{
                return  b.updateTimeNum - a.updateTimeNum
            }))
        }
    }
}
</script>

<style lang="scss">

@import "@/assets/scss/skin.scss";

    .trading-day-header {
        font-size: 10px;
        padding-left: 10px;
    }

    .kf-ajust-order-in-orders-dashboard__warp {

        .mask {
            position: fixed;
            left: 0;
            top: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.5);
            z-index: 99
        }

        .kf-ajust-order-in-orders-dashboard__content {
            position: fixed;
            z-index: 100;
            background: $bg_card;

            .el-input-number.is-without-controls {
                line-height: 20px;

                .el-input {
                    line-height: 20px;
                }

                input.el-input__inner {
                    height: 25px;
                    line-height: 25px;
                    padding: 5px;
                    box-sizing: border-box;

                    ::-webkit-input-placeholder { /* WebKit browsers */
                        font-size: 10px;
                    }

                    ::-moz-placeholder { /* Mozilla Firefox 19+ */
                        font-size: 10px;
                    }

                    :-ms-input-placeholder { /* Internet Explorer 10+ */
                        font-size: 10px;
                    }   
                }
            }
        }
    }
</style>