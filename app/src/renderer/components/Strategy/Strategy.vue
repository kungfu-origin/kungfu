<template>
<tr-dashboard title="策略">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-if="!value">
            <i class="el-icon-monitor mouse-over" title="打开监控" @click="handleMonitStrategies"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item v-else>
            <i class="el-icon-s-platform mouse-over" title="关闭监控" @click="handleMonitStrategies"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-button size="mini" @click="handleAddStrategy" title="添加">添加</el-button>
        </tr-dashboard-header-item>
    </div>
    <div class="table-body">
        <el-table
        :data="tableListAfterFilter"
        @row-click="handleRowClick"
        :row-class-name="handleSelectRow"
        size="small"
        height="100%"
        v-if="afterRender"
        >
            <el-table-column
                prop="strategy_id"
                label="策略"
                sortable
                :show-overflow-tooltip="true"         
                >
            </el-table-column>
            <el-table-column
                label="状态"
                sortable  
                show-overflow-tooltip
            >
                <template slot-scope="props">
                    <tr-status 
                    v-if="ifProcessRunning(props.row.strategy_id, processStatus)"
                    :value="processStatus[props.row.strategy_id]"
                    ></tr-status>
                    <tr-status v-else></tr-status>
                </template>
            </el-table-column>
              <el-table-column
                label="进程"
                sortable    
            >
                <template slot-scope="props" v-if="props.row.strategy_id">
                    <el-switch
                    :value="ifProcessRunning(props.row.strategy_id, processStatus)"
                    @change="e => handleStrategySwitch(e, props.row)"
                    ></el-switch> 
                </template>
            </el-table-column>
            <el-table-column
                class-name="blink-cell"
                label="实现盈亏"
                show-overflow-tooltip
                align="right"
                min-width="100"
                >
                <template slot-scope="props">
                    <span 
                    :class="{
                        'tr-table-cell': true,
                        'number': true,
                        'nano': true,
                        'color-red': calcCash(props.row, 'realizedPnl') > 0,
                        'color-green': calcCash(props.row, 'realizedPnl') < 0,
                    }"
                    :key="`realized_pnl_${props.row.strategy_id}_${calcCash(props.row, 'realizedPnl')}`"                        
                    >
                    {{calcCash(props.row, 'realizedPnl') || '--'}}
                    </span> 
                </template>
            </el-table-column>
            <el-table-column
                class-name="blink-cell"
                label="浮动盈亏"
                show-overflow-tooltip
                align="right"
                min-width="110"
                >
                <template slot-scope="props">
                    <span 
                    :class="{
                        'tr-table-cell': true,
                        'number': true,
                        'nano': true,
                        'color-red': calcCash(props.row, 'unRealizedPnl') > 0,
                        'color-green': calcCash(props.row, 'unRealizedPnl') < 0,
                    }"
                    :key="`unrealized_pnl_${props.row.strategy_id}_${calcCash(props.row, 'unRealizedPnl')}`"                        
                    >
                    {{calcCash(props.row, 'unRealizedPnl') || '--'}}
                    </span> 
                </template>
            </el-table-column>
            <el-table-column
                class-name="blink-cell"
                label="市值"
                show-overflow-tooltip
                align="right"
                min-width="120"
                >
                <template slot-scope="props" >
                    <span 
                    :class="{
                        'tr-table-cell': true,
                        'number': true,
                        'nano': true,
                    }"
                    :key="`${props.row.strategy_id}_${calcCash(props.row, 'marketValue')}`"                        
                    >
                        <template>
                            {{calcCash(props.row, 'marketValue') || '--'}}
                        </template>  
                    </span>          
                </template>
            </el-table-column>
            <el-table-column
                    class-name="blink-cell"
                    label="保证金"
                    show-overflow-tooltip
                    align="right"
                    min-width="120"
                    >
                    <template slot-scope="props" >
                        <span 
                        :class="{
                            'tr-table-cell': true,
                            'number': true,
                            'nano': true,
                        }"
                        :key="`${props.row.strategy_id}_${calcCash(props.row, 'margin')}`"                        
                        >
                            <template>
                                {{calcCash(props.row, 'margin') || '--'}}
                            </template>
                        </span>          
                    </template>
                </el-table-column>
            <el-table-column
                label="路径"
                sortable    
                prop="strategy_path" 
                :show-overflow-tooltip="true"
            >
            </el-table-column>
            <el-table-column
                label="" 
                width="120px"
                align="right"
            >
                <template slot-scope="props">
                    <span class="tr-oper" @click.stop="handleSetStrategy(props.row)"><i class="mouse-over el-icon-setting"></i></span>
                    <span class="tr-oper" @click.stop="handleEditStrategy(props.row)"><i class="mouse-over el-icon-edit-outline"></i></span>
                    <!-- <span class="tr-oper" @click.stop="handleStartAndBacktestWin(props.row)"><i class="mouse-over el-icon-refresh"></i></span> -->
                    <span class="tr-oper-delete" @click.stop="handleDeleteStrategy(props.row)"><i class="mouse-over el-icon-delete"></i></span>
                </template>
            </el-table-column>
        </el-table>
    </div>

    <el-dialog 
    width="340px" 
    :title="setStrategyDialogType == 'add' ? '添加策略' : '设置策略'"  
    v-if="setStrategyDialogVisiblity"
    :visible.sync="setStrategyDialogVisiblity" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleClearAddStrategyDialog"
    @keyup.enter.native="handleConfirmAddEditorStrategy"
    >
        <el-form ref="setStrategyForm" label-width="90px" :model="setStrategyForm">
            <!-- 自定义部分 -->
            <el-form-item
                label="策略ID"
                prop="strategyId"
                :rules="[
                { required: true, message: '请输入策略名称', trigger: 'blur' },
                { min: 1, max: 20, message: '长度不能超过 20 个字符', trigger: 'blur' },
                { validator: validateDuplicateStrategyId, trigger: 'blur' },
                { validator: chineseValidator, trigger: 'blur' },
                { validator: specialStrValidator, trigger: 'blur' },
                { validator: noZeroAtFirstValidator, trigger: 'blur' },
                { validator: noKeywordValidatorBuilder('all'), trigger: 'blur' },
                { validator: noKeywordValidatorBuilder('ledger'), trigger: 'blur' },
                { validator: noKeywordValidatorBuilder('task'), trigger: 'blur' }
                ]"
            >
                <el-input 
                v-model.trim="setStrategyForm.strategyId" 
                :disabled="setStrategyDialogType == 'set'"
                 placeholder="请输入策略名称"
                 ></el-input>
            </el-form-item>
            <el-form-item
            label="入口文件"
            prop="strategyPath"
            :rules="[
                { required: true, message: '请选择策略入口文件路径', trigger: 'blur' },
            ]"
            >
                <span class="path-selection-in-dialog text-overflow" :title="setStrategyForm.strategyPath">{{setStrategyForm.strategyPath}}</span>
                <el-button size="mini" icon="el-icon-more" @click="handleBindStrategyFolder"></el-button>
            </el-form-item>
        </el-form>
        <div slot="footer" class="dialog-footer">
            <el-button  size="mini" @click="handleClearAddStrategyDialog">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleConfirmAddEditorStrategy">确 定</el-button>
        </div>
    </el-dialog>
</tr-dashboard>

</template>
<script>
import { remote } from 'electron';
import { mapState } from 'vuex';

import { ifProcessRunning, toDecimal } from '__gUtils/busiUtils';
import { encodeKungfuLocation } from '__io/kungfu/kungfuUtils';
import { watcher } from '__io/kungfu/watcher';
import * as STRATEGY_API from '__io/kungfu/strategy';
import { switchStrategy } from '__io/actions/strategy';
import { chineseValidator, specialStrValidator, noZeroAtFirstValidator, noKeywordValidatorBuilder } from '__assets/validator';

import baseMixin from '@/assets/js/mixins/baseMixin';

export default {
    mixins: [ baseMixin ],

    props: {
        value: {
            type: Boolean,
            default: false
        }
    },

    data(){

        this.chineseValidator = chineseValidator;
        this.specialStrValidator = specialStrValidator;
        this.noZeroAtFirstValidator = noZeroAtFirstValidator;
        this.noKeywordValidatorBuilder = noKeywordValidatorBuilder;
        this.searchFilterKey = 'strategy_id';
        this.ifProcessRunning = ifProcessRunning;

        return {
            setStrategyDialogVisiblity: false,
            setStrategyDialogType: '',
            setStrategyForm: {
                strategyId: '',
                strategyPath: "",
            },
        }
    },

    beforeMount(){
        this.getStrategyList();
    },

    computed: {
        ...mapState({
            strategyiesAsset: state => state.STRATEGY.strategyiesAsset || {},
            currentStrategy: state => state.STRATEGY.currentStrategy,
            strategyList: state => state.STRATEGY.strategyList,
            processStatus: state => state.BASE.processStatus
        }),
    },

    watch: {
        strategyList (newStrategyList) {
            this.tableList = newStrategyList
        },
    },

    methods: {
        handleMonitStrategies () {
            this.$emit('input', !this.value)
        },

        //绑定策略路径
        handleBindStrategyFolder(){
            const dialog = remote.dialog;
            dialog.showOpenDialog({
                properties: ['openFile']
            }, (strategyPath) => {
                if(!strategyPath || !strategyPath[0]) return;
                this.setStrategyForm.strategyPath = strategyPath[0];
                this.$refs['setStrategyForm'].validate() //手动进行再次验证，因数据放在span中，改变数据后无法触发验证
            })
        },

        //选中行的背景颜色
        handleSelectRow(row) {
            if(row.row.strategy_id == this.currentStrategy.strategy_id) {
                return 'selected-bg'
            }
        },

        //设置当前strategy
        handleRowClick(row) {
            this.$store.dispatch('setCurrentStrategy', row)
        },
        
        //添加策略
        handleAddStrategy() {
            this.setStrategyDialogVisiblity = true;
            this.setStrategyDialogType = 'add'
        },

        //删除策略
        handleDeleteStrategy(row){
            const strategy_id = row.strategy_id + ''
            const strategyProcessStatus = ifProcessRunning(strategy_id, this.processStatus)
            if(strategyProcessStatus){
                this.$message.warning("需先停止该策略运行！")
                return;
            }

            this.$confirm(`删除策略 ${strategy_id} 会删除所有相关信息，确认删除吗？`, '提示', {confirmButtonText: '确 定', cancelButtonText: '取 消'})
            .then(() => this.$store.dispatch('deleteStrategy', strategy_id))
            .then(() => {
                 //如果删除的项是选中的项，则默认选中第一项,如果没有项了，则当前项为空对象{}
                if (this.currentStrategy.strategy_id === strategy_id) {
                    const currentStrategy = this.strategyList.length ? this.strategyList[0] : {}
                    this.$store.dispatch('setCurrentStrategy', currentStrategy)
                }
            })
            .then(() => this.$message.success('操作成功！'))
            .catch((err) => {
                if(err == 'cancel') return
                this.$message.error(err.message || '操作失败！')
            })
        },

        //编辑策略
        handleEditStrategy(row){
            openVueWin(
                'code', 
                `/kungfuCodeEditor/${row.strategy_id}`, 
                remote
            )
        },

        handleStartAndBacktestWin (row) {
            openVueWin('backtest', `/${row.strategy_id}`, BrowserWindow)
        },

        //设置策略
        handleSetStrategy(row){
            this.setStrategyDialogVisiblity = true;
            this.setStrategyDialogType = 'set'
            this.setStrategyForm = {
                strategyId: row.strategy_id,
                strategyPath: row.strategy_path
            }
        },

        //确认添加/编辑策略
        handleConfirmAddEditorStrategy(){
            this.$refs['setStrategyForm'].validate(valid => {
                if(valid){
                    const strategy = this.setStrategyForm.strategyId.toString();
                    const strategyPath = this.setStrategyForm.strategyPath;
                    let firstStepPromise = new Promise(resolve => resolve()) // 添加编辑行为不一样；
                    if(this.setStrategyDialogType === 'add'){
                        firstStepPromise = this.$confirm(`添加后策略ID不能更改，确认添加 ${strategy} 吗？`, '提示', {
                            confirmButtonText: '确 定',
                            cancelButtonText: '取 消',
                        })
                    }
                    firstStepPromise.then(() => {
                        //判断是添加还是修改数据库内容
                        const strategyMethod = this.setStrategyDialogType === 'add' ? STRATEGY_API.addStrategy : STRATEGY_API.updateStrategyPath
                        strategyMethod(strategy, strategyPath)
                        .then(() => this.getStrategyList())//get new list
                        .then(() => {
                            this.$message.success((this.setStrategyDialogType === 'add'? '添加' : '修改') + '成功！')
                            this.handleClearAddStrategyDialog()//clear
                        })
                        .catch((err) => {
                            if(err == 'cancel') return
                            this.$message.error(err.message || '操作失败！')
                        })
                    })
                }else{
                    return false
                }
            })
        },

        //启停策略
        handleStrategySwitch(value, strategy){
            const strategyId = strategy.strategy_id;
            
            if (!value) {
                const strategyLocation = encodeKungfuLocation(strategyId, 'strategy');
                watcher.requestStop(strategyLocation)
            }
         
            switchStrategy(strategyId, value)
                .then(({ type, message }) => this.$message[type](message))
                .catch(err => this.$message['error'](err.message || '操作失败！'))
                
            this.$store.dispatch('getStrategyList');
        },

        //关闭添加strategy弹窗, refresh数据
        handleClearAddStrategyDialog(){
            this.setStrategyForm = { strategyId: '', strategyPath: '' };
            this.setStrategyDialogVisiblity = false;
            this.setStrategyDialogType = ''
        },

        //check策略是否重复
        validateDuplicateStrategyId (rule, value, callback) {
            const ifDuplicate = this.strategyList.filter(s => (s.strategy_id === value)).length !== 0
            if(ifDuplicate && this.setStrategyDialogType !== 'set'){
                callback(new Error('该策略ID已存在！'))
            }else{
                callback()
            }
        },

        //获取策略列表
        getStrategyList(){
            return new Promise(resolve => {
                this.$store.dispatch('getStrategyList').then(strategyList => {
                    resolve(strategyList)
                    //设置第一条为currentStrategy
                    if(!this.currentStrategy.strategy_id){
                        this.$store.dispatch('setCurrentStrategy', strategyList[0] || {})
                    }
                })
            }) 
        },

         //计算持仓盈亏
        calcCash(row, key){
            return toDecimal((this.strategyiesAsset[row.strategy_id] || {})[key]) + ''
        }
    }
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.strategy-mouse{
    &:hover{
        cursor: pointer;
    }
}

.strategy-list .el-input .el-input__inner{
    height: 23px;
    line-height: 23px;
    padding: 0;
}

</style>
