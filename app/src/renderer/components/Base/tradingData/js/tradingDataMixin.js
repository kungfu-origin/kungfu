import moment from 'moment';
import { decodeKungfuLocation } from '__io/kungfu/watcher';
import { history } from '__io/kungfu/kungfuUtils';
import { writeCSV } from '__gUtils/fileUtils';

export default {
    props: {

        currentId: {
            type: String,
            default: ''
        },

        moduleType: {
            type: String,
            default: ''
        },

        accountType: {
            type: String,
            default: ''
        },

        kungfuData: {
            type: Array,
            default: () => {
            }
        },

        addTime: {
            type: Number,
            default: 0
        },

        ifBacktest: {
            type: Boolean,
            default: false
        },

        noTitle: {
            type: Boolean,
            default: false,
        }
    },

    data() {
        return {
            rendererTable: false,
            searchKeyword: "",
            filter: {
                id: '', //对id、标的Id、策略Id模糊查询
            },

            dateRangeDialogVisiblityForExport: false,
            dateRangeDialogVisiblityForHistory: false,
            dateForHistory: '',
            dateRangeExportLoading: false,
            

            tableData: Object.freeze([]),
        }
    },

    mounted() {
        this.rendererTable = true;
        this.resetData();
    },

    computed: {

        schema() {
            return []
        },

        currentTitle () {
            return this.currentId ? `${this.currentId}` : ''
        },
    },

    watch: {
        currentId() {
            this.resetData();
        }
    },

    methods: {

        handleMonitTrades () {
            this.$emit('input', !this.value)
        },

        handleMonitOrders () {
            this.$emit('input', !this.value)
        },

        handleClearHistory () {
            this.dateForHistory = '';
            this.$emit('showHistory', {
                date: '',
                data: [],
                type: this.kungfuBoardType
            })
        },

        handleConfirmDateRangeForHistory (date) {
            return this.getDataByDateRange(date)
                .then(data => {
                    this.dateRangeDialogVisiblityForHistory = false;
                    this.dateForHistory = moment(date).format('YYYY-MM-DD')
                    return data;
                })
                .then(data => {
                    this.$emit('showHistory', {
                        date: date || '',
                        data: data,
                        type: this.kungfuBoardType
                    })
                })
        },

        //选择日期以及保存
        handleConfirmDateRangeForExport (date) {

            return this.getDataByDateRange(date)
                .then(data => {
                    this.dateRangeDialogVisiblityForExport = false;
                    return data;
                })
                .then(data => {
                    const exportTitle = this.kungfuBoardType === 'order' ? '订单' : '成交'
                    this.$saveFile({
                        title: exportTitle,
                    }).then(filename => {
                        if (!filename) return;
                        writeCSV(filename, data)
                    })
                })
        },

        getDataByDateRange (date) {
            const from = moment(date).format('YYYY-MM-DD');
            const to = moment(date).add(1, 'day').format('YYYY-MM-DD');
            this.dateRangeExportLoading = true;

            return new Promise((resolve) => {
                let timer = setTimeout(() => {

                    const kungfuData = history.selectPeriod(from, to)
                    const targetList = this.kungfuBoardType === 'order' ? Object.values(kungfuData.Order) : Object.values(kungfuData.Trade)
                    const kungfuIdKey = this.moduleType === 'account' ? 'source' : 'dest'
                    
                    const targetListAfterFilter = targetList.filter(item => {
                        const locationKey = item[kungfuIdKey];
                        const kungfuLocation = decodeKungfuLocation(locationKey);
                        if (this.moduleType === 'account') {
                            return `${kungfuLocation.group}_${kungfuLocation.name}` === this.currentId
                        } else if (this.moduleType === 'strategy') {
                            return kungfuLocation.name === this.currentId
                        } else if (this.moduleType === 'all') {
                            return true;
                        }

                        return false
                    })

                    this.dateRangeExportLoading = false;

                    resolve(targetListAfterFilter)
                    clearTimeout(timer)
                }, 100)
            })
        },

        resetData() {
            this.searchKeyword = "";
            this.tableData = Object.freeze([]);
            return true;
        },

        renderCellClass(prop, item) {   
            switch (prop) {
                case 'side':
                    if (item.side === '买') return 'red';
                    else if (item.side === '卖') return 'green';
                    break;
                case 'offset':
                    if (item.offset === '开仓') return 'red';
                    else if (item.offset === '平仓') return 'green';
                    break;
                case 'statusName':
                    if (+item.status === 4) return 'red';
                    else if ([3, 5, 6].indexOf(+item.status) !== -1) return 'green';
                    else return 'gray';
                case 'direction':
                    if (item.direction === '多') return 'red';
                    else if (item.direction === '空') return 'green';
                    break;
                case 'realizedPnl':
                    if (+item.realizedPnl > 0) return 'red';
                    else if (+item.realizedPnl < 0) return 'green';
                    break;
                case 'unRealizedPnl':
                    if (+item.unRealizedPnl > 0) return 'red';
                    else if (+item.unRealizedPnl < 0) return 'green';
                    break;
                case 'lastPrice':
                    if (+item.lastPrice > +item.avgPrice) {
                        return item.direction === '多' ? 'red' : 'green';
                    } else if (+item.lastPrice < +item.avgPrice) {
                        return item.direction === '多' ? 'green' : 'red';
                    }
                    break;
                case 'clientId':
                case 'accountId':
                    if (item.clientId.toLowerCase().includes('手动')) return 'blue';
                    break;
            }
        }
    }
}