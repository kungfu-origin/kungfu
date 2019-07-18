<template>
<tr-dashboard title="持仓">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-refresh mouse-over" title="刷新" @click="handleRefresh"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-download mouse-over" title="导出" @click="handleExport"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-button size="mini" @click="makeOrderDialogVisiblity = true">下单</el-button>
        </tr-dashboard-header-item>
    </div>
    <tr-table
        v-if="rendererTable"
        :data="tableData"
        :schema="schema"
        :renderCellClass="renderCellClass"
    ></tr-table>

    <make-order-dialog
        v-if="makeOrderDialogVisiblity"
        :visible.sync="makeOrderDialogVisiblity"
        :moduleType="moduleType"
        :currentId="currentId"
    >
    </make-order-dialog>
</tr-dashboard>
</template>

<script>
import { mapState, mapGetters } from 'vuex'
import { debounce, dealPos } from '__gUtils/busiUtils'
import { ipcRenderer } from 'electron'
import { writeCSV } from '__gUtils/fileUtils';
import MakeOrderDialog from './MakeOrderDialog';



export default {
    name: 'positions',
    props: {
        //传入的id
        currentId: {
            type: String,
            default:''
        },
        //页面类型，是账户页面用还是交易页面用
        moduleType: {
            type: String,
            default:''
        },
        //账户类型，是期货还是股票
        accountType: {
            type: String,
            default:''
        },
        //获取数据的方法名
        getDataMethod: {
            type: Function,
            default: () => {}
        },
        //接收推送的数据
        nanomsgBackData: ''
     
    },

    data() {
        this.accountTypeList = [{
            value: '1',
            label: '股票'
        },{
            value: '2',
            label: '期货'
        },{
            value: '3',
            label: '债券'
        }];
             
        this.posDataByKey = {}; //保存object类型，好在nanomsg推送的时候，找到key

        return {
            rendererTable: false,
            filter: {
                instrumentId: ''  //代码id
            },
            searchKeyword: '',
            getDataLock: false, //防止重复快速的调用
            tableData: Object.freeze([]),

            makeOrderDialogVisiblity: false
        }
    },

    computed:{
        schema() {
            const t = this
            return [{
                    type: 'text',
                    label: '代码',
                    prop: 'instrumentId',
                    width: '70px'
                },{
                    type: 'text',
                    label: '多空',
                    prop: 'direction',
                    width: '50px'
                },{
                    type: 'text',
                    label: '昨',
                    prop: 'yesterdayVolume',
                    flex: 1
                },{
                    type: 'text',
                    label: '今',
                    prop: 'todayVolume',
                    flex: 1
                },{
                    type: 'text',
                    label: '总',
                    prop: 'totalVolume',
                    flex: 1
                },{
                    type: 'text',
                    label: '开仓均价',
                    prop: 'openPrice',
                    flex: 1.2
                },{
                    type: 'text',
                    label: '最新价',
                    prop: 'lastPrice',
                    flex: 1
                },{
                    type: 'text',
                    label: '浮动盈亏',
                    prop: 'unRealizedPnl',
                    flex: 1.2
                }
            ]
        }
    },

    components: {
        MakeOrderDialog
    },


    watch: {
        //防抖
        searchKeyword: debounce(function (value) {
            this.filter.instrumentId = value
        }),

        currentId(val) {
            const t = this;
            t.resetData();
            if(!val) return;
            t.rendererTable = false;
            t.$nextTick().then(() => {
                t.rendererTable = true;
                t.init()
            })
        },

        filter:{
            deep: true,
            handler() {
                const t = this;
                t.currentId && t.init(true)
            }
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
        t.currentId && t.init()
    },

    destroyed(){
        ipcRenderer.removeAllListeners(`res-cancel-order-rate-${this.moduleType}`)
    },
    
    methods:{
        handleRefresh(){
            const t = this;
            t.resetData();
            t.currentId && t.init();
        },

        handleExport(){
            const t = this;
            t.$saveFile({
                title: '保存持仓信息',
            }).then(filename => {
                if(!filename) return;
                writeCSV(filename, t.tableData)
            })
        },

        init: debounce(function() {
            const t = this;
            t.getData()
        }),

        //重置数据
        resetData() {
            const t = this
            t.filter = {
                type: '',
                instrumentId: ''
            };
            t.searchKeyword = '';
            t.getDataLock = false;
            t.tableData = Object.freeze([]);
            return true;
        },

        //获取数据
        getData() {
            const t = this
            if(t.getDataLock) throw new Error('get data lock！');
            t.getDataLock = true
            t.tableData = []
            t.posDataByKey = {}
            return new Promise((resolve, reject) => {
                t.getDataMethod(t.currentId, t.filter).then(res => {
                    if(!res){
                        resolve(Object.freeze([]))
                        return;
                    }
                    let tableData = []
                    res.map(item => {
                        const dealItem = dealPos(item)
                        tableData.push(dealItem)
                        const key = t.getKey(item)//key并非ticker
                        t.posDataByKey[key] = dealItem
                    })
                    t.tableData = Object.freeze(tableData) 
                    resolve(t.posDataByKey)
                }).finally(() => {
                    t.getDataLock = false
                })
            })
        },

        dealNanomsg(data) {
            const t = this
            //如果存在筛选，则推送的数据也需要满足筛选条件
            if(!data.instrument_id.includes(t.filter.instrumentId)) return
            const poskey = t.getKey(data)
            t.posDataByKey[poskey] = { ...dealPos(data) }
            //更新数据, 根据ID来排序
            const sortPosList = Object.values(t.posDataByKey).sort((a, b) =>{
                return a.instrumentId - b.instrumentId
            })
            //更新数据
            t.tableData = Object.freeze(sortPosList)
        },

        //拼接key值
        getKey(data) {
            return (data.instrument_id + data.direction)
        },

        renderCellClass(prop, item){
            switch(prop){
                case 'direction':
                    if(item.direction === '多') return 'red'
                    else if(item.direction === '空') return 'green'
                    break
                case 'realizedPnl':
                    if(item.realizedPnl > 0) return 'red'
                    else if(item.realizedPnl < 0) return 'green'
                    break
                case 'unRealizedPnl':
                    if(item.unRealizedPnl > 0) return 'red'
                    else if(item.unRealizedPnl < 0) return 'green'
                    break
                case 'lastPrice':
                    if(item.lastPrice > item.openPrice) return 'red'
                    else if(item.lastPrice < item.openPrice) return 'green'
                    break;
            }
        }
    }
}
</script>

<style lang="scss">
.positions{
    height: 100%;
}
</style>