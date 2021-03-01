<template>
    <el-dialog
        width="450px"
        :visible="visible" 
        :title="`${method === 'add'? '添加' : '设置'} 标的池`"  
        :close-on-click-modal="false"
        :close-on-press-escape="true"
        @close="handleClose"
        @keyup.enter.native="handleSubmitSetting"
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
                                { validator: validateDuplicateStrategyId, trigger: 'blur' },
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
            <el-button type="primary" size="mini" @click="handleSetTickerSet">确 定</el-button>
        </div>
    </el-dialog>
</template>

<script>

import { specialStrValidator, noZeroAtFirstValidator, noKeywordValidatorBuilder } from '__assets/validator';

export default {

    props: {
        visible: {
            type: Boolean,
            default: false
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

            tickersList: []
        }
    },

    methods: {
        handleDeleteTicker () {},

        handleSetTickerSet () {
            
        },

        handleAddTicker () {},

        handleClose () {
            this.$emit('update:visible', false);
        },

          //check是否重复
        validateDuplicateStrategyId(rule, value, callback){
            const ifDuplicate = this.strategyList.filter(s => (s.strategy_id === value)).length !== 0
            if(ifDuplicate && this.setStrategyDialogType !== 'set'){
                callback(new Error('该标的池名称已存在！'))
            }else{
                callback()
            }
        },

        handleSubmitSetting () {},
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