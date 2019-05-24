<template>
    <tr-dashboard title="交易账户">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <tr-search-input v-model.trim="accountIdKey"></tr-search-input>
            </tr-dashboard-header-item>
            <tr-dashboard-header-item>
                <el-button size="mini" @click="handleAddAccount" title="添加">添加</el-button>
            </tr-dashboard-header-item>
        </div>
        <div class="table-body">
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
                    label="账户"
                    show-overflow-tooltip
                >
                    <template slot-scope="props">
                        {{props.row.account_id.toAccountId()}}
                    </template>
                </el-table-column>
                <el-table-column
                    prop="source_name"
                    label="柜台"
                    >
                    <template slot-scope="props">
                        <el-tag
                        v-if="(config[props.row.source_name] || {}).typeName"
                        :type="config[props.row.source_name].type" 
                        >
                            {{props.row.source_name}}
                        </el-tag>
                    </template>
                </el-table-column>
                <el-table-column
                    label="状态"
                    show-overflow-tooltip
                    >
                    <template slot-scope="props">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('td_' + props.row.account_id, processStatus)"
                        :value="(mdTdState[`td_${props.row.account_id}`] || {}).state"></tr-status>
                        <tr-status v-else></tr-status>
                    </template>
                </el-table-column>
                    
                <el-table-column
                    label="连接"
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
                    label="累计盈亏"
                    show-overflow-tooltip
                    >
                    <template slot-scope="props">
                        <span :class="{
                            'color-red': calcAccumulatedPnl(props.row) > 0,
                            'color-green': calcAccumulatedPnl(props.row) < 0,
                        }">
                        {{calcAccumulatedPnl(props.row) || '--'}}
                        </span> 
                    </template>
                </el-table-column>
                <el-table-column
                    label="累计盈亏率"
                    show-overflow-tooltip
                    >
                    <template slot-scope="props">
                        <span :class="{
                            'color-red': calcAccumulatedPnlRate(props.row) > 0,
                            'color-green': calcAccumulatedPnlRate(props.row) < 0,
                        }">
                        {{calcAccumulatedPnlRate(props.row) + '' ? calcAccumulatedPnlRate(props.row) + '%' : '--'}}
                        </span> 
                    </template>
                </el-table-column>
                <el-table-column
                    label="市值/保证金"
                    show-overflow-tooltip
                    >
                    <template slot-scope="props" >
                        <template v-if="(config[props.row.source_name] || {}).typeName == 'future'">
                            {{$utils.toDecimal((accountsAsset[props.row.account_id] || {}).margin) + '' || '--'}}
                        </template>
                        <!-- market_value -->
                         <template v-else>
                            {{$utils.toDecimal((accountsAsset[props.row.account_id] || {}).market_value) + '' || '--'}}
                        </template>
                    </template>
                </el-table-column>
                <el-table-column
                    label="可用资金"
                    show-overflow-tooltip
                    >
                        <template slot-scope="props">
                        {{$utils.toDecimal((accountsAsset[props.row.account_id] || {}).avail) + '' || '--'}}
                    </template>
                </el-table-column>
                <el-table-column
                    label=""
                    align="right"
                    min-width="100"
                >
                    <template slot-scope="props">
                        <span class="tr-oper" @click.stop="handleOpenLogFile(props.row)"><i class="el-icon-document mouse-over" title="打开日志文件"></i></span>
                        <span class="tr-oper" @click.stop="handleOpenFeeSettingDialog(props.row)"><i class="el-icon-money mouse-over" title="费率设置"></i></span>
                        <span class="tr-oper" @click.stop="handleOpenUpdateAccountDialog(props.row)"><i class="el-icon-setting mouse-over" title="账户设置"></i></span>
                        <span class="tr-oper-delete" @click.stop="handleDeleteAccount(props.row)"><i class=" el-icon-delete mouse-over" title="删除账户"></i></span>
                    </template>
                </el-table-column>
            </el-table>
        </div>
         <!-- 选择柜台 -->
            <el-dialog title="选择柜台" 
            width="400px" 
            :visible.sync="visiblity.selectSource"
            >
                    <el-radio-group v-model.trim="selectedSource" style="width: 100%">
                        <el-row>
                            <el-col :span="12" v-for="item of Object.values(config)" :key="item.platform">
                                <el-radio :label="item.source" :disabled="ifSourceDisable[item.source.toLowerCase()]">
                                    {{item.source.toUpperCase()}}
                                    <el-tag
                                    v-if="item.typeName"
                                    :type="item.type" 
                                    >
                                        {{item.typeName === 'future' ? '期货' : '股票'}}
                                    </el-tag>
                                </el-radio>
                            </el-col>
                        </el-row>
                    </el-radio-group>
                <div slot="footer" class="dialog-footer">
                    <el-button @click="handleCloseSelectSource" size="mini">取 消</el-button>
                    <el-button type="primary" size="mini" @click="handleSelectSource">确 定</el-button>
                </div>
            </el-dialog>

            <!-- 设置账户 -->
            <SetAccountDialog
            v-model.trim="accountForm"
            v-if="visiblity.setAccount"
            :visible.sync="visiblity.setAccount"
            :method="method" 
            :source="selectedSource"
            @successSubmitSetting="successSubmitSetting"
            @refreshData="refreshData"
            :firstAccount="sourceFirstAccount"
            :accountList="accountList"
            />

            <SetFeeDialog
            v-if="visiblity.setFee"
            :visible.sync="visiblity.setFee"
            :accountType="(config[feeAccount.source_name] || {}).typeName"
            :accountId="feeAccount.account_id"
            :setFeeSettingData="setFeeSettingData"
            :getFeeSettingData="getFeeSettingData"
            ></SetFeeDialog>
    </tr-dashboard>
</template>

<script>
import {mapState, mapGetters} from 'vuex'
import {debounce} from '@/assets/js/utils'
import * as ACCOUNT_API from '@/io/account'
import * as BASE_API from '@/io/base'
import {accountSource, sourceType, ifSourceDisable} from '@/assets/config/accountConfig'
import SetAccountDialog from './SetAccountDialog'
import SetFeeDialog from './SetFeeDialog'
import {deleteProcess} from '__gUtils/processUtils'
import {ACCOUNTS_DIR, LOG_DIR, buildGatewayPath} from '__gConfig/pathConfig'
import {removeFileFolder, openReadFile} from "__gUtils/fileUtils.js"

import path from 'path'
export default {
    name: 'account',

    data() {
        this.config = sourceType;
        this.ifSourceDisable = ifSourceDisable;
        return {
            accountIdKey: '',
            selectedExecutor: '',
            accountIdSearchKeyDebounce: '',
            method: 'add',
            accountForm: {},
            selectedSource: '',
            visiblity: {
                selectSource: false,
                setAccount: false,
                setFee: false,
            },
            sourceFirstAccount: false, //来标记是否是某柜台下添加的第一个账户
            taskList: [], //存放kungfu_task数据表内容
            renderTable: false, //table等到mounted后再渲染，不然会导致table高度获取不到，页面卡死
            // processStatus: Object.freeze({}),
            setFeeSettingData: ACCOUNT_API.setFeeSettingData,
            getFeeSettingData: ACCOUNT_API.getFeeSettingData,
            feeAccount: null
        }
    },

    components: {
        SetAccountDialog,
        SetFeeDialog
    },

    computed:{
        ...mapState({
            mdTdState: state => state.ACCOUNT.mdTdState,
            accountsAsset: state => state.ACCOUNT.accountsAsset,
            accountList: state => state.ACCOUNT.accountList, 
            currentAccount: state => state.ACCOUNT.currentAccount,
            currentId: state => (state.ACCOUNT.currentAccount || {}).account_id,
            processStatus: state => state.BASE.processStatus
        }),

        //用来存放筛选完的列表
        accountFilter() {
            const t = this
            let accounts = []
            if(!t.accountIdSearchKeyDebounce) return t.accountList;
            return t.accountList.filter(a => (a.account_id.indexOf(t.accountIdSearchKeyDebounce) !== -1));
        },
    },
    watch: {
        //防抖
        accountIdKey: debounce(function (value) {
            this.accountIdSearchKeyDebounce = value
        }),
    },

    beforeMount(){
        const t = this;
        t.getAccountList();
        t.$store.dispatch('getTasks')
    },

    mounted() {
        const t = this;
        t.renderTable = true
    },

    methods:{
        //添加账户，打开选择柜台弹窗
        handleAddAccount(){
            this.visiblity.selectSource = true
        },

        //编辑账户
        handleOpenUpdateAccountDialog(row) {
            const t = this
            t.method = 'update'
            t.accountForm = JSON.parse(row.config) 
            t.selectedSource = row.source_name
            t.visiblity.setAccount = true
        },

        //删除账户信息
        handleDeleteAccount(row) {
            const t = this
            if(!t.judgeCondition(row)) return
            const accountId = row.account_id.toAccountId()
            t.$confirm(`删除账户${accountId}会删除所有相关信息，确认删除吗？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => t.deleteAccount(row))
            .then(() => t.getAccountList())
            .then(() => {
                //如果删除的项是选中的项，则默认选中第一项,如果没有项了，则当前项为空对象{}
                if(t.currentId == row.account_id) {
                    const currentAccount = t.accountList.length ? t.accountList[0] : {}
                    t.$store.dispatch('setCurrentAccount', currentAccount)
                }else if(!t.accountList.length){
                    t.$store.dispatch('setCurrentAccount', {})
                }
            })
            .then(() => deleteProcess('td_' + row.account_id))
            .then(() => (row.receive_md) && deleteProcess('md_' + row.source_name))
            .then(() => t.$message.success('操作成功！'))
            .catch((err) => {
                if(err == 'cancel') return
                t.$message.error(err.message || '操作失败！')
            })
        },

        //费率设置
        handleOpenFeeSettingDialog(row){
            const t = this;
            t.visiblity.setFee = true;
            t.feeAccount = row;
        },

        //选择柜台
        handleSelectSource() {
            const t = this
            t.method = 'add';
            if(!t.selectedSource) {
                t.$message.warning('还没有选择柜台！')
                return;
            };
            //是否是该柜台下的第一个账户记住，行情自动选中
            t.sourceFirstAccount = -1 === t.accountList.findIndex(item => (item.source_name == t.selectedSource))
            // 加上某些参数的默认值
            accountSource[t.selectedSource].map(item => {
                if(item.default !== undefined) {
                    t.accountForm[item.key] = item.default
                }
            })
            t.visiblity.selectSource = false
            t.visiblity.setAccount = true
        },

        //关闭选择柜台弹窗
        handleCloseSelectSource() {
            const t = this
            t.visiblity.selectSource = false
            t.refreshData()
        },

        //选中行的背景颜色
        handleSelectRow(row) {
            const t = this;
            if(row.row.account_id == t.currentId) {
                return 'selected-bg'
            }
        },

        //当前行高亮
        handleRowClick(row) {
            const t = this;
            t.$store.dispatch('setCurrentAccount', row)
        },

        //Td开关
        handleTdSwitch(value, account) {
            const t = this;
            t.$store.dispatch('switchTd', {
                account,
                value
            })
        },

        //打开日志
        handleOpenLogFile(row){
            const logPath = path.join(LOG_DIR, `td_${row.account_id}.log`);
            openReadFile(logPath)
        },

        // updateProcessStatus(res){
        //     const t = this;
        //     t.processStatus = res
        // },

        //获取账户列表
        getAccountList() {
            const t = this
            return new Promise(resolve => {
                t.$store.dispatch('getAccountList').then(accountList => {
                    resolve(accountList)
                    if(!t.currentId){
                        t.$store.dispatch('setCurrentAccount', accountList[0] || {})
                    }
                })
            })   
        },

        //删除账户需要将所关联的数据库以及进程都关掉
        //判断task表和进程中是否存在，有则删除
        //TODO
        deleteAccount(row) {
            const t = this
            const {account_id, source_name, receive_md} = row
            //查看该账户下是否存在task中的td任务
            const tdProcessId = `td_${account_id}`
            const mdProcessId = `md_${source_name}`

            //删除td
            return removeFileFolder(path.join(ACCOUNTS_DIR, account_id.toAccountId()))
            .then(() => t.$store.dispatch('deleteTask', tdProcessId))                      
            .then(() => ACCOUNT_API.deleteAccount(account_id))//删除账户表中的数据
            .then(() => removeFileFolder(buildGatewayPath(tdProcessId)))
            .then(() => t.getAccountList())
            .then(accountList => t.removeMdGatewayFolder(accountList, mdProcessId, source_name, receive_md))
        },

        //删除md的gateway folder
        removeMdGatewayFolder(accountList, mdProcessId, source_name, receive_md){
            const t = this;
            const accountNotMdBySource = accountList.filter(a => (a.source_name == source_name && a.receive_md === 0))            
            const accountBySource = accountList.filter(a => (a.source_name == source_name)) 
            if(!accountBySource.length) {
                removeFileFolder(buildGatewayPath(mdProcessId))
                t.$store.dispatch('deleteTask', mdProcessId)
            }
            if(accountNotMdBySource.length && receive_md) return ACCOUNT_API.changeAccountMd(accountNotMdBySource[0].account_id, true)
        },

        //删除前进行一些判断
        judgeCondition(row) {
            const t = this
            const {account_id, source_name, receive_md} = row

            //判断td是否开启，开启则无法删除
            if(t.$utils.ifProcessRunning('td_' + account_id, t.processStatus)) {
                t.$message.warning('需先停止交易运行！')
                return false
            }

            //判断是否是md行情源,是则看md是否是开启状态，开启则需要先删除
            if(receive_md && t.$utils.ifProcessRunning('md_' + source_name, t.processStatus)) {
                t.$message.warning('当前账户为柜台行情源，需先停止柜台行情运行！')
                return false
            }
            return true
        },

        //添加或修改账户详情后的操作
        successSubmitSetting() {
            const t = this
            t.getAccountList()
            t.refreshData()
        },

        //清空数据
        refreshData() {
            const t = this
            t.selectedSource = ''
        },

        //计算持仓盈亏
        calcAccumulatedPnl(row){
            const t = this;
            return t.$utils.toDecimal((t.accountsAsset[row.account_id] || {}).accumulated_pnl) + ''
        },

        //计算持仓盈亏率
        calcAccumulatedPnlRate(row){
            const t = this;
            return t.$utils.toDecimal((t.accountsAsset[row.account_id] || {}).accumulated_pnl_ratio, 6, 2) 
        }
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';
.table-list{
    .source-name{
        span.name{
            color: #fff;
        }
    }
    .fr .el-switch, .fr .tr-dot-content{
        margin-left: 10px;
    }
}

.account-source{
    line-height: 30px;
    padding-left: 10px;
    >div{
        display: inline-block;
        margin-right: 8px;
        span:first-child{
            padding-right: 4px;
        }
    }
}
</style>