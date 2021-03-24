<template>
  <tr-dashboard :title="currentTitleResolved">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>

        <tr-dashboard-header-item v-if="!dateForHistory">
            <i class="el-icon-date mouse-over" title="历史" @click="dateRangeDialogVisiblityForHistory = true"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-else>
            <span>{{ dateForHistory }}</span>
            <i class="el-icon-close mouse-over" @click="handleClearHistory"></i>
        </tr-dashboard-header-item>

        <tr-dashboard-header-item v-if="todayFinishPreSetting === undefined">
            <i class="el-icon-success mouse-over"  v-if="todayFinish" title="查看未完成委托" @click="todayFinish = false"></i>
            <i class="el-icon-circle-check mouse-over" v-else title="查看全部委托" @click="todayFinish = true"></i>
        </tr-dashboard-header-item>

        <tr-dashboard-header-item>
            <i class="el-icon-download mouse-over" title="导出" @click="dateRangeDialogVisiblityForExport = true"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
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

import DatePickerDialog from '@/components/Base/DatePickerDialog';

import { dealOrder } from "__io/kungfu/watcher";
import { kungfuCancelAllOrders } from '__io/kungfu/makeCancelOrder';
import { aliveOrderStatusList } from 'kungfu-shared/config/tradingConfig';
import { ordersHeader } from '@/components/Base/tradingData/js/tableHeaderConfig';

import makeOrderMixin from '@/components/Base/makeOrder/js/makeOrderMixin';
import makeOrderCoreMixin from '@/components/Base/makeOrder/js/makeOrderCoreMixin';
import tradingDataMixin from '@/components/Base/tradingData/js/tradingDataMixin';

export default {
    name: "orders",
   
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

        todayFinishPreSetting: {
            type: [ Object, Boolean ],
            default: () => undefined
        }
    },

    data () {
        return {
            todayFinish: this.todayFinishPreSetting !== undefined ? this.todayFinishPreSetting : true,
            kungfuBoardType: 'order',
        };
    },

    components: {
        DatePickerDialog,
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus
        }),

        gatewayNameResolved () {
            if (this.gatewayName) return `td_${this.gatewayName}`;
            else return ''
        },

        schema () {
            return ordersHeader(this.dateForHistory, this.moduleType)
        },

        currentTitleResolved () {
            if (this.noTitle) {
                return ''
            }

            if (this.todayFinish) {
                return `全部委托 ${this.currentTitle}`
            } else {
                return `未完成委托 ${this.currentTitle}`
            }
        },
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

            orderData.accountId = orderData.accountId.split('手动').join(' ').split('任务').join(' ').trim()
            
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

        //对返回的数据进行处理
        dealOrderList (orders, { searchKeyword, todayFinish }) {
            let orderDataByKey = {};
            let ordersAfterFilter = orders
                .filter(item => {
                    if (searchKeyword.trim() === '') return true;
                    const { client_id, source_id, account_id, instrument_id } = item
                    const strings = [ client_id, source_id, account_id, instrument_id ].join('')
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