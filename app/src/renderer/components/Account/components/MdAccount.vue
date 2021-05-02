<template>
      <tr-dashboard title="">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <el-button size="mini" @click="handleToggleKeepAllProcessRunning" :title="allProcessBtnTxt">{{ allProcessBtnTxt }}</el-button>
            </tr-dashboard-header-item>
            <tr-dashboard-header-item>
                <el-button size="mini" @click="handleAdd" title="添加" id="add-account-btn">添加</el-button>
            </tr-dashboard-header-item>
        </div>
        <div class="table-body md-table">
            <el-table
            size="small"
            :data="mdList"
            height="100%"
            v-if="afterRender"
            >
                <el-table-column
                    prop="source_name"
                    label="柜台"
                    show-overflow-tooltip
                >
                    <template slot-scope="props">
                        <el-tag
                        v-if="(mdAccountSource[props.row.source_name] || {}).typeName"
                        :type="mdAccountSource[props.row.source_name].type" 
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
                        v-if="ifProcessRunning(`md_${props.row.source_name}`, processStatus) && processStatus[`md_${props.row.source_name}`] === 'online'"
                        :value="(mdTdState[`md_${props.row.source_name}`] || {}).state"></tr-status>
                        <tr-status 
                        v-else-if="ifProcessRunning(`md_${props.row.source_name}`, processStatus) && processStatus[`md_${props.row.source_name}`] === 'stopping'"
                        :value="processStatus[`md_${props.row.source_name}`]"></tr-status>
                        <tr-status v-else></tr-status>
                    </template>
                </el-table-column>
                    
                <el-table-column
                    label="进程"
                    >
                    <template slot-scope="props">
                        <span @click.stop>
                            <el-switch :value="ifProcessRunning('md_' + props.row.source_name, processStatus)" @change="handleMdSwitch($event, props.row)"></el-switch>
                        </span>
                    </template>
                </el-table-column>
                <el-table-column
                    label=""
                    align="right"
                    min-width="120"
                >
                    <template slot-scope="props">
                        <span class="tr-oper" @click.stop="handleOpenLogFile(`md_${props.row.source_name}`)"><i class="el-icon-document mouse-over" title="打开日志"></i></span>
                        <span class="tr-oper" @click.stop="handleOpenUpdateAccountDialog(props.row)"><i class="el-icon-setting mouse-over" title="MD 设置"></i></span>
                        <span :class="['tr-oper-delete', `delete-${props.row.source_name}`] " @click.stop="handleDeleteMd(props.row)"><i class=" el-icon-delete mouse-over" title="删除 MD"></i></span>
                    </template>
                </el-table-column>
            </el-table>
        </div>

        <!-- 选择柜台 -->
        <SetSourceDialog
        v-if="visiblity.selectSource"
        :visible.sync="visiblity.selectSource"
        :accountSource="mdAccountSource"
        @confirm="handleSelectSource"
        @noAvailSources="handleNoAvailSource"
        :existMdList="mdList"
        />

        <!-- 设置账户 -->
        <SetAccountDialog
        :value="accountForm"
        v-if="visiblity.setAccount"
        :visible.sync="visiblity.setAccount"
        :method="method" 
        :source="selectedSource"
        :accountSource="mdAccountSource"
        @successSubmitSetting="successSubmitSetting"
        @refreshData="refreshData"
        :mdTdList="mdList"
        type="md"
        />
    </tr-dashboard>
</template>

<script>
import { mapState } from 'vuex';

import SetAccountDialog from './SetAccountDialog';
import SetSourceDialog from './SetSourceDialog';

import { getMdList } from '__io/kungfu/account';
import { switchMd, deleteMd } from '__io/actions/account';
import { loopToRunProcess, ifProcessRunning } from '__gUtils/busiUtils';

import baseMixin from '@/assets/mixins/baseMixin';
import mdTdMixin from '../js/mdTdMixin';
import openLogMixin from '@/assets/mixins/openLogMixin';

export default {
    mixins: [ baseMixin, mdTdMixin, openLogMixin ],

    data () {
        this.tdmdType = 'md';
        this.ifProcessRunning = ifProcessRunning;

        return {

        }
    },

    computed: {
        ...mapState({
            mdAccountSource: state => state.BASE.mdAccountSource || {},
            mdList: state => state.ACCOUNT.mdList,
            mdTdState: state => state.ACCOUNT.mdTdState,
        }),

        allProcessRunning () {
            const notRunningList = this.mdList.filter(item => {
                const isRunning = ifProcessRunning('md_' + item.source_name, this.processStatus)
                if (!isRunning) return true
                else return false
            })

            if (notRunningList.length) return false;
            return true
        },
    },

    components: {
        SetAccountDialog,
        SetSourceDialog
    },

    methods: {

        //删除账户信息
        handleDeleteMd(row) {
            if(!this.judgeCondition(row)) return;
            const { source_name } = row
            //查看该账户下是否存在task中的td任务
            const mdProcessId = `md_${source_name}`
            this.$confirm(`删除行情源${source_name}会删除所有相关信息，确认删除吗？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => deleteMd(row, this.tdList))
            .then(() => this.getTableList())
            .then(() => this.$message.success('操作成功！'))
            .catch(err => {
                if(err == 'cancel') return
                this.$message.error(err.message || '操作失败！')
            })
        },

        handleMdSwitch(value, account) {
            return switchMd(account, value).then(({ type, message }) => this.$message[type](message))  
        },

        handleNoAvailSource(bool) {
            if(bool) {
                this.$message.info('行情源都已添加！')
            }
        },

        switchAllProcess (targetStatus) {
            const promiseList = this.mdList
                .filter(item => {
                    const id = item.source_name;
                    const status = ifProcessRunning('md_' + item.source_name, this.processStatus)
                    return status !== targetStatus
                })
                .map(item => {
                    return () => switchMd(item, targetStatus)
                })

            if (this.ifMasterLedgerRunning && this.watcherIsLive) {
                return loopToRunProcess(promiseList)
            } else {
                return Promise.resolve(false)
            }
        },

         //删除前进行一些判断
        judgeCondition(row) {
            const { source_name } = row
            //判断td是否开启，开启则无法删除
            if(ifProcessRunning(`md_${source_name}`, this.processStatus)) {
                this.$message.warning('需先停止行情源进程！')
                return false
            }
            return true
        },

        //获取账户列表
        getTableList() {
            return getMdList()
                .then(res => {
                    this.$store.dispatch('setMdList', res || [])
                }).catch(err => {
                    this.$message.error(err.message || '操作失败！')
                })
        },
    }
    
    
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.md-table{
   height: 100%;
}

</style>





