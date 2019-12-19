<template>
      <tr-dashboard title="行情源">
        <div slot="dashboard-header">
            <tr-dashboard-header-item>
                <el-button size="mini" @click="handleAdd" title="添加" id="add-account-btn">添加</el-button>
            </tr-dashboard-header-item>
        </div>
        <div class="md-source">
            <el-table
            size="small"
            :data="mdList"
            height="100%"
            v-if="renderTable"
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
                        v-if="$utils.ifProcessRunning('md_' + props.row.source_name, processStatus)"
                        :value="(mdTdState[`md_${props.row.source_name}`] || {}).state"></tr-status>
                        <tr-status v-else></tr-status>
                    </template>
                </el-table-column>
                    
                <el-table-column
                    label="连接"
                    >
                    <template slot-scope="props">
                        <span @click.stop>
                            <el-switch :value="$utils.ifProcessRunning('md_' + props.row.source_name, processStatus)" @change="handleMdSwitch($event, props.row)"></el-switch>
                        </span>
                    </template>
                </el-table-column>
                <el-table-column
                    label=""
                    align="right"
                >
                    <template slot-scope="props">
                        <span class="tr-oper" @click.stop="handleOpenLogFile(props.row)"><i class="el-icon-document mouse-over" title="打开日志"></i></span>
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
import path from 'path';
import Vue from 'vue';
import { mapState, mapGetters } from 'vuex';
import * as ACCOUNT_API from '__io/db/account';
import { LOG_DIR } from '__gConfig/pathConfig';
import { switchMd, deleteMd } from '__io/actions/account';

import SetAccountDialog from './SetAccountDialog';
import SetSourceDialog from './SetSourceDialog';

import mdTdMixin from '../js/mdTdMixin';

export default {
    mixins: [ mdTdMixin ],

    computed: {
        ...mapState({
            mdAccountSource: state => state.BASE.mdAccountSource || {},
            mdList: state => state.ACCOUNT.mdList,
            mdTdState: state => state.ACCOUNT.mdTdState,
            processStatus: state => state.BASE.processStatus
        })
    },

    components: {
        SetAccountDialog,
        SetSourceDialog
    },

    methods: {

        //删除账户信息
        handleDeleteMd(row) {
            const t = this
            if(!t.judgeCondition(row)) return;
            const { source_name } = row
            //查看该账户下是否存在task中的td任务
            const mdProcessId = `md_${source_name}`
            t.$confirm(`删除行情源${source_name}会删除所有相关信息，确认删除吗？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => deleteMd(row, t.tdList))
            .then(() => t.getTableList())
            .then(() => t.$message.success('操作成功！'))
            .catch((err) => {
                if(err == 'cancel') return
                t.$message.error(err.message || '操作失败！')
            })
        },

        handleMdSwitch(value, account) {
            const t = this
            switchMd(account, value).then(({ type, message }) => t.$message[type](message))  
        },

        handleOpenLogFile(row){
            const logPath = path.join(LOG_DIR, `md_${row.source_name}.log`);
            this.$showLog(logPath)
        },

        handleNoAvailSource(bool) {
            const t = this;
            if(bool) {
                t.$message.info('行情源都已添加！')
            }
        },

         //删除前进行一些判断
        judgeCondition(row) {
            const t = this
            const { source_name } = row
            //判断td是否开启，开启则无法删除
            if(t.$utils.ifProcessRunning(`md_${source_name}`, t.processStatus)) {
                t.$message.warning('需先停止行情源进程！')
                return false
            }
            return true
        },

        //获取账户列表
        getTableList() {
            const t = this;
            ACCOUNT_API.getMdList().then(res => {
                if(!res) return;
                t.$store.dispatch('setMdList', res)
            }).catch(err => {
                t.$message.error(err.message || '操作失败！')
            })
        },
    }
    
    
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.md-source{
   height: 100%;
}

</style>





