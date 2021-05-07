<template>
    <el-dialog
        width="450px"
        :visible="visible" 
        :title="`${method === 'add'? '添加' : '设置'} 标的池`"  
        :close-on-click-modal="false"
        :close-on-press-escape="true"
        @close="handleClose"
        @keyup.enter.native="handleConfirmTickerSet"
    >
        <div class="ticker-set-dialog__warp">
            <div class="name-content">
                <tr-dashboard title="标的池名称">
                    <el-form ref='setTickerSetNameForm' :model="setTickerSetNameForm">
                        <el-form-item
                            label=""
                            prop="name"
                            :rules="[
                                { min: 1, max: 20, message: '长度不能超过 20 个字符', trigger: 'blur' },
                                { required: true, message: '请输入标的池名称', trigger: 'blur' },
                                { validator: validateDuplicateTickerSet, trigger: 'blur' },
                                { validator: specialStrValidator, trigger: 'blur' },
                                { validator: noZeroAtFirstValidator, trigger: 'blur' },
                            ]"
                        >
                            <el-input 
                            v-model.trim="setTickerSetNameForm.name" 
                            :disabled="method == 'set'"
                            placeholder="请输入标的池名称"
                            ></el-input>
                        </el-form-item>
                    </el-form>
                </tr-dashboard>
               
            </div>

            <div class="tickers-table-content">
                <tr-dashboard title="标的列表">
                    <div slot="dashboard-header">
                        <tr-dashboard-header-item>
                            <el-button size="mini" @click="handleAddTicker" title="添加" id="add-account-btn">添加</el-button>
                        </tr-dashboard-header-item>
                    </div>
                    <div class="table-body tickers-table">
                         <el-table
                            size="small"
                            :data="tickersList"
                            height="100%"
                        >
                            <el-table-column
                                prop="instrumentId"
                                label="标的"
                                show-overflow-tooltip
                                min-width="80"
                            ></el-table-column>
                            <el-table-column
                                prop="exchangeId"
                                label="交易所"
                                show-overflow-tooltip
                                min-width="60"
                            ></el-table-column>
                            <el-table-column
                                prop="source"
                                label="柜台"
                                show-overflow-tooltip
                                min-width="60"
                            >
                                <template slot-scope="props">
                                    <el-tag
                                    v-if="(tdAccountSource[props.row.source] || {}).typeName"
                                    :type="tdAccountSource[props.row.source].type" 
                                    >
                                        {{props.row.source}}
                                    </el-tag>
                                </template>
                            </el-table-column>
                               <el-table-column
                                    label="" 
                                    width="60"
                                    align="right"
                                >
                                    <template slot-scope="props">
                                        <span class="tr-oper-delete" @click.stop="handleDeleteTicker(props.row)"><i class="mouse-over el-icon-delete"></i></span>
                                    </template>
                                </el-table-column>
                        </el-table>
                    </div> 
                </tr-dashboard>
            </div>
        </div>
        
        <div slot="footer" class="dialog-footer">
            <el-button  size="mini" @click="handleClose">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleConfirmTickerSet">确 定</el-button>
        </div>

    </el-dialog>
</template>

<script>
import { mapState } from 'vuex';

import { deepClone, getIndexFromTargetTickers } from '__gUtils/busiUtils';
import { specialStrValidator, noZeroAtFirstValidator, noKeywordValidatorBuilder } from '__assets/validator';

export default {

    props: {
        visible: {
            type: Boolean,
            default: false
        },

        inputData: {
            type: Object,
            default: {}
        },

        method: {
            type: String,
            default: 'add'
        }
    },

  
    data () {
        this.specialStrValidator = specialStrValidator;
        this.noZeroAtFirstValidator = noZeroAtFirstValidator;
        this.noKeywordValidatorBuilder = noKeywordValidatorBuilder;

        return {
            setTickerSetNameForm: {
                name: ''
            },

            tickersList: [],

        }
    },

    mounted () {
        this.bindAddTickerListener();

        if (this.method !== 'add') {
            this.initData();
        }
    },

    beforeDestroy () {
        this.$bus.$off('add-ticker-for-ticker-set')
    },

    computed: {

        ...mapState({
            tickerSets: state => state.MARKET.tickerSets,
            tdAccountSource: state => state.BASE.tdAccountSource || {}
        })
    },

    methods: {

        handleDeleteTicker (tickerData) {
            const targetIndex = getIndexFromTargetTickers(this.tickersList, tickerData)
            if (targetIndex !== -1) {
                this.tickersList.splice(targetIndex, 1)
            }
        },

        handleConfirmTickerSet () {
            this.$refs['setTickerSetNameForm'].validate(valid => {
                if (valid) {
                    this.$emit('confirm', {
                        name: this.setTickerSetNameForm.name,
                        tickers: deepClone(this.tickersList)
                    })
                    this.handleClose()
                }
            })
        },

        handleAddTicker () {
            this.$emit('addTicker')
        },

        handleClose () {
            this.$emit('update:visible', false);
            this.$emit('close')
        },

        bindAddTickerListener () {
            this.$bus.$on('add-ticker-for-ticker-set', (tickerData) => {
                const targetIndex = getIndexFromTargetTickers(this.tickersList, tickerData)
                if (targetIndex === -1) {
                    this.tickersList.push(tickerData)
                } else {
                    this.tickersList.splice(targetIndex, 1, tickerData)
                }
            })
        },

        initData () {
            const { name, tickers } = this.inputData;
            this.$set(this.setTickerSetNameForm, 'name', name);
            this.tickersList = deepClone(tickers)
        },

        //check是否重复
        validateDuplicateTickerSet (rule, value, callback ) {
            const ifDuplicate = this.tickerSets.filter(s => (s.name === value)).length !== 0
            if(ifDuplicate && this.method === 'add'){
                callback(new Error('该标的池名称已存在！'))
            }else{
                callback()
            }
        },
    }
}
</script>

<style lang="scss">

.ticker-set-dialog__warp {
    
    .name-content {
        height: 100px;
    }

    .tickers-table {
        height: 200px;
    }
}

</style>