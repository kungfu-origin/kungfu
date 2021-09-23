import moment from 'moment';
import { decodeKungfuLocation, transformOrderStatListToData, dealOrderStat } from '@kungfu-trader/kungfu-uicc/io/kungfu/watcher';
import { getKungfuDataByDateRange } from '@kungfu-trader/kungfu-uicc/io/kungfu/kungfuUtils';
import { writeCSV } from '@kungfu-trader/kungfu-uicc/utils/fileUtils';
import { getDefaultRenderCellClass, originOrderTradesFilterByDirection } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';

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
            default: () => ([])
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

        currentIdResolved () {
            if (this.moduleType === 'account') {
                return this.currentId
            } else if (this.moduleType === 'strategy') {
                return this.currentId
            } else if (this.moduleType === 'ticker') {
                const { instrumentId, directionOrigin } = this.currentTicker || {};
                return `${instrumentId}_${directionOrigin}`
            } else {
                return ''
            }
        }
    },

    watch: {
        currentIdResolved() {
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
            this.dateRangeExportLoading = true;

            return getKungfuDataByDateRange(date)
                .then(kungfuData => {
                    const { instrumentId, directionOrigin } = this.currentTicker || {};
                    const targetList = this.getHistoryTargetListResolved(this.kungfuBoardType, kungfuData, this.moduleType, instrumentId);
                    const orderStats = kungfuData.OrderStat.list();
                    const orderStatByOrderId = transformOrderStatListToData(orderStats);
                    return targetList
                        .filter(item => {
                            if (this.moduleType === 'account') {
                                return this.getHistoryDataKeyForFilter('account', item) === this.currentId;
                            } else if (this.moduleType === 'strategy') {
                                return this.getHistoryDataKeyForFilter('strategy', item) === this.currentId;
                            } else if (this.moduleType === 'all') {
                                return true;
                            } else if (this.moduleType === 'ticker') {
                                const { offset, side, instrument_type } = item;
                                return originOrderTradesFilterByDirection(directionOrigin, offset, side, instrument_type);
                            }
                        })
                        .map(item => {
                            //加上orderStat细节
                            const orderId = item.order_id.toString();
                            return Object.freeze({
                                //for export
                                ...orderStatByOrderId[orderId],
                                //for trades/orders recoard components
                                orderStats: dealOrderStat(orderStatByOrderId[orderId] || null),
                                ...item,
                                dest: item.dest,
                                source: item.source,
                                tag: item.tag,
                                ts: item.ts,
                                type: item.type,
                                uid_key: item.uid_key
                            })
                        });                    
                })
                .finally(() => {
                    this.dateRangeExportLoading = false;
                    this.dateRangeDialogVisiblityForExport = false;
                    this.dateRangeDialogVisiblityForHistory = false;
                })
        },

    
        
        getHistoryTargetListResolved (kungfuBoardType, kungfuData, moduleType = '' , instrumentId = '') {
            const kfDataTable = this.getHistoryTargetList(kungfuBoardType, kungfuData);
            const sortName = this.getHistoryTargetSortName(kungfuBoardType);
            if (moduleType === 'ticker') {
                return kfDataTable.filter('instrument_id', instrumentId).sort(sortName);
            } else {
                return kfDataTable.sort(sortName);
            }
        },

        getHistoryTargetList (kungfuBoardType, kungfuData ) {
            if (kungfuBoardType === 'order') {
                return kungfuData.Order
            } else if (kungfuBoardType === 'trade') {
                return kungfuData.Trade
            } else {
                console.error('getHistoryTargetList type is not trade or order!')
                return []
            }
        },

        getHistoryTargetSortName (kungfuBoardType) {
            if (kungfuBoardType === 'order') {
                return 'update_time'
            } else if (kungfuBoardType === 'trade') {
                return 'trade_time'
            } else {
                return ''
            }
        },

        getHistoryDataKeyForFilter (moduleType, item) {
            if (moduleType === 'account') {
                const kungfuLocation = decodeKungfuLocation(+item.source);
                return `${kungfuLocation.group}_${kungfuLocation.name}`
            } else if (moduleType === 'strategy') {
                const kungfuLocation = decodeKungfuLocation(+item.dest);
                return kungfuLocation.name
            } else {
                console.error('getHistoryDataKeyForFilter type is not account or strategy!')
                return []
            }
        },

        resetData() {
            this.searchKeyword = "";
            this.tableData = Object.freeze([]);
            this.handleClearHistory();
            return true;
        },

        renderCellClass(prop, item) {   
            return getDefaultRenderCellClass(prop, item)
        }
    }
}