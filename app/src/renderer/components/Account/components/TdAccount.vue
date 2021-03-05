<template>
    <tr-dashboard title="">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <tr-search-input v-model.trim="accountIdKey"></tr-search-input>
            </tr-dashboard-header-item>
            <tr-dashboard-header-item>
                <el-button size="mini" @click="handleToggleKeepAllProcessRunning" :title="allProcessBtnTxt">{{ allProcessBtnTxt }}</el-button>
            </tr-dashboard-header-item>
            <tr-dashboard-header-item>
                <el-button size="mini" @click="handleAdd" title="添加" id="add-account-btn">添加</el-button>
            </tr-dashboard-header-item>
        </div>
        <div class="table-body accounts-table">
            <el-table
                v-if="renderTable"
                size="small"
                :data="accountFilter"
                height="100%"
                @row-click="handleRowClick"
                :row-class-name="handleSelectRow"
                >
                  <el-table-column
                    prop="account_id"
                    label="名称"
                    show-overflow-tooltip
                    min-width="80"
                >
                    <template slot-scope="props">
                        <span>{{JSON.parse(props.row.config).account_name || '-'}}</span> 
                    </template>
                </el-table-column>
                <el-table-column
                    prop="account_id"
                    label="账户"
                    show-overflow-tooltip
                    min-width="80"
                >
                    <template slot-scope="props">
                        <span :class="props.row.account_id">{{props.row.account_id.toAccountId()}}</span> 
                    </template>
                </el-table-column>
                <el-table-column
                    prop="source_name"
                    label="柜台"
                    min-width="80"
                    >
                    <template slot-scope="props">
                        <el-tag
                        v-if="(tdAccountSource[props.row.source_name] || {}).typeName"
                        :type="tdAccountSource[props.row.source_name].type" 
                        >
                            {{props.row.source_name}}
                        </el-tag>
                    </template>
                </el-table-column>
                <el-table-column
                    label="状态"
                    show-overflow-tooltip
                    min-width="80"
                    >
                    <template slot-scope="props">
                        <tr-status 
                        v-if="$utils.ifProcessRunning(`td_${props.row.account_id}`, processStatus) && processStatus[`td_${props.row.account_id}`] === 'online'"
                        :value="(mdTdState[`td_${props.row.account_id}`] || {}).state"></tr-status>
                        <tr-status 
                        v-else-if="$utils.ifProcessRunning(`td_${props.row.account_id}`, processStatus) && processStatus[`td_${props.row.account_id}`] === 'stopping'"
                        :value="processStatus[`td_${props.row.account_id}`]"></tr-status>
                        <tr-status v-else></tr-status>
                    </template>
                </el-table-column>
                    
                <el-table-column
                    label="进程"
                    min-width="60"
                    >
                    <template slot-scope="props">
                        <span @click.stop>
                        <el-switch 
                        :value="$utils.ifProcessRunning('td_' + props.row.account_id, processStatus)"
                        @change="handleTdSwitch($event, props.row)"></el-switch>
                        </span>
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
                        :key="`realized_pnl_${props.row.account_id}_${calcCash(props.row, 'realizedPnl')}`"                        
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
                        :key="`unrealized_pnl_${props.row.account_id}_${calcCash(props.row, 'unRealizedPnl')}`"                        
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
                        :key="`marketValue_${props.row.account_id}_${calcCash(props.row, 'marketValue')}`"                        
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
                        :key="`margin_${props.row.account_id}_${calcCash(props.row, 'margin')}`"                        
                        >
                            <template>
                                {{calcCash(props.row, 'margin') || '--'}}
                            </template> 
                        </span>          
                    </template>
                </el-table-column>
                <el-table-column
                    class-name="blink-cell"
                    label="可用资金"
                    show-overflow-tooltip
                    align="right"
                    min-width="120"
                    >
                        <template slot-scope="props">
                            <span
                            :class="{
                                'tr-table-cell': true,
                                'number': true,
                                'nano': true
                            }"
                            :key="`${props.row.account_id}_${calcCash(props.row, 'avail')}`"                        
                            >
                                {{calcCash(props.row, 'avail') || '--'}}                            
                            </span>
                    </template>
                </el-table-column>
                <el-table-column
                    label=""
                    align="right"
                    min-width="120"
                >
                    <template slot-scope="props">
                        <span class="tr-oper" @click.stop="handleOpenLogFile(`td_${props.row.account_id}`)"><i class="el-icon-document mouse-over" title="打开日志文件"></i></span>
                        <span class="tr-oper" @click.stop="handleOpenUpdateAccountDialog(props.row)"><i class="el-icon-setting mouse-over" title="TD 设置"></i></span>
                        <span :class="['tr-oper-delete', `delete-${props.row.account_id}`] " @click.stop="handleDeleteTd(props.row)"><i class=" el-icon-delete mouse-over" title="删除 TD"></i></span>
                    </template>
                </el-table-column>
            </el-table>
        </div>
         <!-- 选择柜台 -->
        <SetSourceDialog
        :visible.sync="visiblity.selectSource"
        :accountSource="tdAccountSource"
        @confirm="handleSelectSource"
        />

        <!-- 设置账户 -->
        <SetAccountDialog
        :value="accountForm"
        v-if="visiblity.setAccount"
        :visible.sync="visiblity.setAccount"
        :method="method" 
        :source="selectedSource"
        :accountSource="tdAccountSource"
        @successSubmitSetting="successSubmitSetting"
        @refreshData="refreshData"
        :mdTdList="tdList"
        type="td"
        />

    </tr-dashboard>
</template>

<script>
import { mapState } from 'vuex';

import SetAccountDialog from './SetAccountDialog';
import SetSourceDialog from './SetSourceDialog';

import { debounce } from '__gUtils/busiUtils';
import { getTdList } from '__io/kungfu/account';
import { TD_DIR } from '__gConfig/pathConfig';
import { removeFileFolder } from "__gUtils/fileUtils";
import { deleteTd, switchTd } from '__io/actions/account';
import { loopToRunProcess } from '__gUtils/busiUtils';
import { watcher } from '__io/kungfu/watcher';

import mdTdMixin from '../js/mdTdMixin';
import openLogMixin from '@/assets/js/mixins/openLogMixin';

import path from 'path'
export default {
    name: 'account',

    mixins: [ mdTdMixin, openLogMixin ],

    data() {
        this.tdmdType = 'td';

        return {
            accountIdKey: '',
            accountIdSearchKeyDebounce: '',
        }
    },

    components: {
        SetAccountDialog,
        SetSourceDialog
    },

    computed:{
        ...mapState({
            tdAccountSource: state => state.BASE.tdAccountSource || {},
            mdTdState: state => state.ACCOUNT.mdTdState || {},
            accountsAsset: state => state.ACCOUNT.accountsAsset || {},
            tdList: state => state.ACCOUNT.tdList || [], 
            currentAccount: state => state.ACCOUNT.currentAccount || {},
        }),

        currentId () {
            return this.currentAccount.account_id || ''
        },

        //用来存放筛选完的列表
        accountFilter() {
            let accounts = []
            if(!this.accountIdSearchKeyDebounce) return this.tdList;
            return this.tdList.filter(a => (a.account_id.includes(this.accountIdSearchKeyDebounce)));
        },

        allProcessRunning () {
             const notRunningList = this.tdList.filter(item => {
                const isRunning = this.$utils.ifProcessRunning('td_' + item.account_id, this.processStatus)
                if (!isRunning) return true
                else return false
            })

            if (notRunningList.length) return false;
            return true
        },
    },
    watch: {
        //防抖
        accountIdKey: debounce(function (value) {
            this.accountIdSearchKeyDebounce = value
        }),
    },

    methods:{
       
        //删除账户信息
        handleDeleteTd(row) {
            if(!this.judgeCondition(row)) return;
            const { account_id } = row
            //查看该账户下是否存在task中的td任务
            const tdProcessId = `td_${account_id}`
            const accountId = account_id.toAccountId()
            this.$confirm(`删除账户${accountId}会删除所有相关信息，确认删除吗？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => deleteTd(row, this.tdList))
            .then(() => this.getTableList())
            .then(() => {
                //如果删除的项是选中的项，则默认选中第一项,如果没有项了，则当前项为空对象{}
                if(this.currentId == row.account_id) {
                    const currentAccount = this.tdList.length ? this.tdList[0] : {}
                    this.$store.dispatch('setCurrentAccount', currentAccount)
                }else if(!this.tdList.length){
                    this.$store.dispatch('setCurrentAccount', {})
                }
            })
            .then(() => this.$message.success('操作成功！'))
            .catch((err) => {
                if(err == 'cancel') return
                this.$message.error(err.message || '操作失败！')
            })
        },

        //选中行的背景颜色
        handleSelectRow(row) {
            if(row.row.account_id == this.currentId) {
                return 'selected-bg'
            }
        },

        //当前行高亮
        handleRowClick(row) {
            this.$store.dispatch('setCurrentAccount', row)
        },

        //Td开关
        handleTdSwitch(value, account) {
            return switchTd(account, value).then(({ type, message }) => this.$message[type](message))
        },

        switchAllProcess (targetStatus) {
            const promiseList = this.tdList
                .filter(item => {
                    const id = item.account_id;
                    const status = this.$utils.ifProcessRunning('td_' + item.account_id, this.processStatus)
                    return status !== targetStatus
                })
                .map(item => {
                    return () => switchTd(item, targetStatus)
                })
            
            if (this.ifMasterLedgerRunning && watcher.isLive) {
                return loopToRunProcess(promiseList)
            } else {
                return Promise.resolve(false)
            }
        },

        //获取账户列表
        getTableList() {
            return getTdList()
                .then(tdList => {
                    this.$store.dispatch('setTdList', tdList);
                    if(!this.currentId) this.$store.dispatch('setCurrentAccount', tdList[0] || {})
                    return tdList
                })
                .catch(err => {
                    this.$message.error(err.message || '操作失败！')
                })
        },

        //删除前进行一些判断
        judgeCondition(row) {
            const { account_id } = row
            //判断td是否开启，开启则无法删除
            if(this.$utils.ifProcessRunning(`td_${account_id}`, this.processStatus)) {
                this.$message.warning('需先停止交易进程！')
                return false
            }
            return true
        },

        //是否为期货
        isFutureAccount(row) {
            console.log(this.tdAccountSource)
            return (this.tdAccountSource[row.source_name] || {}).typeName == 'future'
        },

        //计算持仓盈亏
        calcCash(row, key){
            return this.$utils.toDecimal((this.accountsAsset[row.account_id] || {})[key]) + ''
        }
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.table-list{
    .source-name {
        span.name {
            color: #fff;
        }
    }
    .fr .el-switch, .fr .tr-dot-content {
        margin-left: 10px;
    }
}

.account-source {
    line-height: 30px;
    padding-left: 10px;
    >div {
        display: inline-block;
        margin-right: 8px;
        span:first-child{
            padding-right: 4px;
        }
    }
}

#select-source-dialog {
    .el-col {
        padding-right: 20px;
    } 
    .el-tag {
        vertical-align: bottom
    }
}
</style>