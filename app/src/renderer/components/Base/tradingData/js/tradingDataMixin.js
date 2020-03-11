
export default {
    props: {
        currentId: {
            type: String,
            default:''
        },
        
        moduleType: {
            type: String,
            default:''
        },
        
        kungfuData: {
            type: Array,
            default: () => {}
        },

        tradingDay: {
            type: String,
            default: ''
        },

        addTime: {
            type: Number,
            default: 0
        }
    },
    
    data () {
        return {
            rendererTable: false,
            searchKeyword: "",
            filter: {
                id: '', //对id、代码id、策略id模糊查询
                dateRange: null 
            },
            dateRangeDialogVisiblity: false,
            tableData: Object.freeze([])
        }
    },

    mounted () {
        this.rendererTable = true;
        this.resetData();
    },

    computed: {

        schema () {
            return []
        },

        currentTitle () {
            return this.currentId ? `${this.currentId}` : ''
        },
    },

    watch: {
        currentId() {
            this.resetData();
        },

        tradingDay () {
            this.resetData();
        },
    },

    methods: {
        handleRefresh(){
            this.resetData();
        },

        //选择日期以及保存
        handleConfirmDateRange(dateRange){
            const t = this;
            console.error('请求历史数据 TODO')
        },

        resetData() {
            const t = this;
            t.todayFinish = true;
            t.searchKeyword = "";
            t.tableData = Object.freeze([]);
            return true;
        },

        renderCellClass (prop, item) {
            switch(prop){
                case 'side':
                    if (item.side === '买') return 'red'
                    else if (item.side === '卖') return 'green'
                    break
                case 'offset':
                    if (item.offset === '开仓') return 'red'
                    else if (item.offset === '平仓') return 'green'
                    break
                case 'statusName':
                    if (item.statusName === '错误') return 'red'
                    else if (['已成交', '部分撤单', '已撤单'].indexOf(item.statusName) != -1) return 'green'
                    else return 'gray'
                case 'direction':
                    if (item.direction === '多') return 'red'
                    else if (item.direction === '空') return 'green'
                    break
                case 'realizedPnl':
                    if (+item.realizedPnl > 0) return 'red'
                    else if (+item.realizedPnl < 0) return 'green'
                    break
                case 'unRealizedPnl':
                    if (+item.unRealizedPnl > 0) return 'red'
                    else if (+item.unRealizedPnl < 0) return 'green'
                    break
                case 'lastPrice':
                    if (+item.lastPrice > +item.avgPrice) return 'red'
                    else if (+item.lastPrice < +item.avgPrice) return 'green'
                    break;
                case 'clientId':
                case 'accountId':
                    if (item.clientId.toLowerCase().includes('手动')) return 'blue'
                    break

            }
        }
    }
}