<template>
    <div class="kf-ticker-set__warp">
        <tr-dashboard title="标的池列表">
            <div slot="dashboard-header">
                <tr-dashboard-header-item>
                    <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
                </tr-dashboard-header-item>
                <tr-dashboard-header-item>
                    <el-button size="mini" @click="handleAddTickerSet" title="添加">添加标的池</el-button>
                </tr-dashboard-header-item>
            </div>
            <div class="table-body ticker-set-table">
                <el-table
                    size="small"
                    :data="tableListAfterFilter"
                    height="100%"
                    @row-click="handleClickRow"
                    :row-class-name="handleSelectRow"
                >
                    <el-table-column
                        label="名称"
                        show-overflow-tooltip
                    >
                        <template slot-scope="props">
                            <span 
                            v-if="props.row.status !== 'editing'" 
                            title="双击重命名" 
                            @dblclick.stop="handleChangeName(props.row)">
                                {{ props.row.name }}
                            </span> 
                            <span 
                            v-if="props.row.status !== 'editing' && props.row.name === currentTickerSetName" 
                            title="双击重命名" 
                            @dblclick.stop="handleChangeName(props.row)">
                                (当前)
                            </span>
                            <el-input 
                            ref="editing-tickerset-input"
                            v-if="props.row.status === 'editing' && props.row.editingType === 'new'"
                            :autofocus="true"
                            v-model="tmpTickerSetName"
                            @blur="handleBlurEditingNewTickerSet(props.$index)"
                            ></el-input>
                            <el-input 
                            ref="editing-tickerset-input"
                            v-if="props.row.status === 'editing' && props.row.editingType === 'rename'"
                            :autofocus="true"
                            v-model="tmpTickerSetName"
                            @blur="handleBlurEditingNewTickerSet(props.$index)"
                            ></el-input>
                        </template>
                    </el-table-column>
                    <el-table-column
                        prop="tickers"
                        label="内含标的"
                        show-overflow-tooltip
                    >
                        <template slot-scope="props">
                            {{ (props.row.tickers || []).map(ticker => ticker.instrumentId).join(', ')  }}
                        </template>
                    </el-table-column>
                    <el-table-column
                        label=""
                        align="right"
                        width="100px"
                    >
                    <template slot-scope="props">
                        <span class="tr-oper-edit" v-if="props.row.name !== 'editing'" ><i class=" el-icon-edit mouse-over" title="重命名" @click.stop="handleChangeName(props.row)"></i></span>
                        <span class="tr-oper-delete" v-if="props.row.name !== 'editing'" ><i class=" el-icon-delete mouse-over" title="删除" @click.stop="handleRemoveTickerSet(props.row)"></i></span>
                    </template>
                    </el-table-column>
                </el-table>
            </div>
        </tr-dashboard>
    </div>
</template>

<script>

import { findTargetFromArray } from '__gUtils/busiUtils';
import baseMixin from '@/assets/mixins/baseMixin';
import tickerSetMixin from '@/components/MarketFilter/js/tickerSetMixin';
import { retry } from 'rxjs/operators';


export default {
    mixins: [ baseMixin, tickerSetMixin ],

    name: 'TickerSet',

    props: {
        editingTickerSet: Object
    },

    data () {
        this.searchFilterKey = 'name, tickers';
        return {
            tmpTickerSetName: ''
        }
    },

    mounted () {
        this.initEditingTickerSet()
    },
  
    watch: {
        tickerSets (newTickerSets) {
            this.tableList = newTickerSets
                .slice(0)
                .sort((item1, item2) => {
                    return (item1.name || '').localeCompare(item2.name || '');
                })
                .map(item => {
                    return {
                        ...item,
                        status: '',
                        editingType: ''
                    }
                });
            const editingTickerSetName = (this.editingTickerSet || {}).name || '';
            
            if (!this.tableList.length) {
                this.updateEditingTickerSet(null)
            } else if (!findTargetFromArray(this.tableList, 'name', editingTickerSetName)) {
                //after deleted
                if (this.tableList.length) {
                    this.updateEditingTickerSet(this.tableList[0])
                }
            } 
        },
    },

    methods: {
        handleAddTickerSet () {
            if (!findTargetFromArray(this.tableList, 'name', 'editing')) {
                this.tableList.push({
                    status: "editing",
                    editingType: 'new'
                })

                this.focusInput()
            }
        },

        handleChangeName (row) {
            this.$set(row, 'status', 'editing');
            this.$set(row, 'editingType', 'rename');
            this.tmpTickerSetName = row.name;

            this.focusInput();

        },

        handleBlurEditingNewTickerSet (index) {

            const targetTicker = this.tableListAfterFilter[index] || {};
            const tmpTickerSetName = this.tmpTickerSetName;
            this.tmpTickerSetName = "";  
            
            if (targetTicker.editingType !== 'new' && targetTicker.editingType !== 'rename') return;

            if (!tmpTickerSetName) {
                if (this.targetTicker.editingType === 'rename') {
                    this.$set(targetTicker, 'status', '')
                    this.$set(targetTicker, 'editingType', '')
                } else {
                    this.tableList.pop();
                }
                return
            };

            if (targetTicker.editingType === 'new') {
                if (findTargetFromArray(this.tableList, 'name', tmpTickerSetName)) {
                    this.$message.warning(`标的池 ${tmpTickerSetName} 已存在！`);
                    return 
                } 
            }

            //new
            if (targetTicker.editingType === 'new') {
                    //delete editing 
                this.tableList.pop();
                const tickerSet = {
                    name: tmpTickerSetName,
                    tickers: []
                }
                return this.handleConfirmAddSetTickerSet(tickerSet)
                    .then(() => this.updateEditingTickerSet(tickerSet))
            } 
            
            //rename
            const newTickerSet = {
                name: tmpTickerSetName,
                tickers: targetTicker.tickers || []
            }

            return this.handleRemoveTickerSet(targetTicker, true)
                .then(() => this.handleConfirmAddSetTickerSet(newTickerSet))
                .then(() => this.updateEditingTickerSet(newTickerSet))
                .then(() => {
                    //需要同时更新 vuex state currentTickerSet
                    if (this.currentTickerSet.name === targetTicker.name) {
                        this.handleSetCurrentTickerSet(newTickerSet)
                    }
                })
                .finally(() => {
                    this.$set(targetTicker, 'status', '')
                    this.$set(targetTicker, 'editingType', '')
                })
        },

        handleClickRow (row) {
            this.updateEditingTickerSet(row)
        },

        //选中行的背景颜色
        handleSelectRow (row) {
            if(row.row.name == (this.editingTickerSet || {}).name) {
                return 'selected-bg'
            }
        },

        focusInput () {
            return this.$nextTick()
                .then(() => {
                    const $elInput = this.$refs['editing-tickerset-input'];
                    if ($elInput) {
                        $elInput.$el.querySelectorAll('input')[0].focus();
                    }
                })
        },

        initEditingTickerSet () {
            if (this.editingTickerSet !== null) return;
            if (this.tickerSets && this.tickerSets.length) {
                this.updateEditingTickerSet(this.tickerSets[0])
            }
        },

        updateEditingTickerSet (tickerSet) {
            this.$emit('update:editingTickerSet', Object.freeze(tickerSet))
        },

        resolveTickers (tickers) {
            return tickers.map(ticker => {
                return `${ticker.instrumentId}`
            }).join(',')
        },      
    }
}
</script>

<style lang="scss">

.kf-ticker-set__warp {
    height:  100%;

    
    .ticker-set-table {
        height: 100%
    }
    
}


</style>