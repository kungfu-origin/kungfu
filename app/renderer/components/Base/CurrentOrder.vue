<template>
  <tr-dashboard :title="filter.dateRange ? '委托记录':'未完成委托'">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="fa fa-refresh mouse-over" title="刷新" @click="handleRefresh"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="fa fa-download mouse-over" title="导出" @click="handleRefresh"></i>
        </tr-dashboard-header-item>
         <tr-dashboard-header-item>
            <el-button size="mini" type="danger" style="color: #fff" title="全部撤单">CANCEL</el-button>
        </tr-dashboard-header-item>
        <!-- <tr-dashboard-header-item width="199px"> -->
            <!-- <el-date-picker
                v-model.trim="filter.dateRange"
                size="mini"
                type="daterange"
                range-separator="～"
                start-placeholder="开始日期"
                end-placeholder="结束日期">
            </el-date-picker> -->            
        <!-- </tr-dashboard-header-item> -->
    </div>
    <tr-table
    v-if="rendererTable"
    :data="tableData"
    :schema="schema"
    :renderCellClass="renderCellClass"
    ></tr-table>
  </tr-dashboard>
</template>

<script>
import moment from "moment"
import { offsetName, orderStatus, sideName } from "@/assets/config/tradingConfig";
import { debounce, throttle } from "@/assets/js/utils";
export default {
    name: "current-orders",
    props: {
        currentId: {
            type: String,
            default:''
        },
        pageType: {
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
            tableData: Object.freeze([])
        };
    },

    computed:{
            schema(){
                return  [{
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
                    prop: "status",
                    },{
                    type: "text",
                    label: this.pageType == 'account'?'策略': '账户',
                    prop: this.pageType == 'account'? 'clientId': 'accountId',
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
        t.currentId && t.init()
    },

    methods: {
        handleRefresh(){
            const t = this;
            t.resetData();
            t.currentId && t.init()
        },

        init: debounce(function() {
            const t = this
            t.$emit('startNanomsg');
            t.getData();
        }),


        getData() {
            const t = this
            if(t.getDataLock) throw new Error('get-data-lock')
            t.getDataLock = true
            //clear Data
            t.orderDataByKey = {};
            t.tableData = []
            return t.getDataMethod(t.currentId, t.filter)
            .then(res => {
                if(!res.data) {
                    t.tableData = Object.freeze([]);                    
                    return;
                }
                const {tableData, orderDataByKey}  = t.dealData(res.data);
                t.tableData = Object.freeze(tableData);
                t.orderDataByKey = orderDataByKey;  
            }).catch(err => {
            }).finally(() => {
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
                const dealItem = t.dataOrder(item)
                tableData.push(dealItem)
                orderDataByKey[item.order_id] = dealItem
            })
            return {tableData, orderDataByKey}
        },  

        //处理需要的数据及顺序
        dataOrder(item) {
            const t = this;
            return Object.freeze({
                id: item.order_id.toString() + '_' + item.account_id.toString(),
                insertTime: item.insert_time && moment(item.insert_time/1000000).format("YYYY-MM-DD HH:mm:ss"),
                instrumentId: item.instrument_id,
                side: sideName[item.side] ? sideName[item.side] : '--',
                offset: offsetName[item.offset] ? offsetName[item.offset] : '--',
                limitPrice: item.limit_price,
                volumeTraded: item.volume_traded + "/" + (item.volume),
                status: orderStatus[item.status],
                clientId: item.client_id,
                accountId: item.account_id,
                orderId: item.order_id,
            })
        },

        resetData() {
            const t = this;
            t.searchKeyword = "";
            t.filter = {
                id: '',
                dateRange: null
            };
            t.tableData = Object.freeze([]);
            t.getDataLock = false;
            return true;
        },

        handleDownload(){},

        //对nanomsg推送回来的数据进行处理
        dealNanomsg(data) {
            const t = this
            //当有日期筛选的时候，不需要推送数据
            if(!!t.filter.dateRange) return
            const {order_id, status, client_id} = data
            //策略页面收到的数据判断该数据是否是本策略的
            if(t.pageType == 'strategy' && t.currentId != client_id ) return
            const {id} = t.filter
            //推送的时候也要满足筛选条件
            if(!(order_id.toString().includes(id) || data.instrument_id.includes(id) || data.client_id.includes(id))) return

            //status为3,4,5,6的时候不显示在当前委托中
            if([3, 4, 5, 6].indexOf(+status) !== -1) {
                if(!t.orderDataByKey[order_id]) return
                t.orderDataByKey[order_id] = null;
                delete t.orderDataByKey[order_id]
            }else {
                t.orderDataByKey[order_id] = t.dataOrder(data)
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
            if(prop === 'status'){
                if(item.status === '错误') return 'red'
                else if(['已成交', '部分撤单', '已撤单'].indexOf(item.status) != -1) return 'green'
                else return 'gray'
            }
        }
    }
}
</script>

<style lang="scss">
</style>