
import moment from 'moment';

import { decodeKungfuLocation } from '__gUtils/kungfuUtils';
import { history } from '__gUtils/kungfuUtils';
import { writeCSV } from '__gUtils/fileUtils';

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
            const from = moment(dateRange[0]).format('YYYYMMDD')
            const to = moment(dateRange[1]).format('YYYYMMDD')
            
            this.$nextTick()
                .then(() => {
                    const kungfuData = history.selectPeriod(from, to)
                    const targetList = this.kungfuBoardType === 'order' ? Object.values(kungfuData.Order) : Object.values(kungfuData.Trade)
                    const kungfuIdKey = this.moduleType === 'account' ? 'source' : 'dest'
                    const exportTitle = this.kungfuBoardType === 'order' ? '订单' : '成交'

                    const targetListAfterFilter = targetList.filter(item => {
                        const locationKey = item[kungfuIdKey];
                        const kungfuLocation = decodeKungfuLocation(locationKey);

                        if (this.moduleType === 'account') {
                            return `${kungfuLocation.group}_${kungfuLocation.name}` === this.currentId
                        } else if (this.moduleType === 'strategy') {
                            return kungfuLocation.name === this.currentId
                        }
                        
                        return false                
                    })

                    this.$saveFile({
                        title: exportTitle,
                    }).then(filename => {
                        if(!filename) return;
                        writeCSV(filename, targetListAfterFilter)
                    })
                })
        },

        resetData() {
            this.todayFinish = true;
            this.searchKeyword = "";
            this.tableData = Object.freeze([]);
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