<template>
<tr-dashboard title="">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-button size="mini" @click="handleAddTask" title="添加">添加</el-button>
        </tr-dashboard-header-item>
    </div>
    <div class="table-body">
        <el-table
        :data="tableListAfterFilter"
        size="small"
        height="100%"
        v-if="afterRender"
        >
            <el-table-column
                prop="processId"
                label="任务ID"
                sortable
                :show-overflow-tooltip="true"       
                width="249px"  
                >
            </el-table-column>
            <el-table-column
                label="状态"
                sortable  
                show-overflow-tooltip
            >
                <template slot-scope="props">
                    <tr-status 
                    v-if="$utils.ifProcessRunning(props.row.processId, processStatus)"
                    :value="props.row.processStatus"
                    ></tr-status>
                    <tr-status v-else></tr-status>
                </template>
            </el-table-column>
            <el-table-column
                label="运行"
                sortable    
            >
                <template slot-scope="props">
                    <el-switch
                    :value="$utils.ifProcessRunning(props.row.processId, processStatus)"
                    @change="e => handleTaskSwitch(e, props.row)"
                    ></el-switch> 
                </template>
            </el-table-column>
            <el-table-column
                label="创建时间"
                sortable  
                prop="createdAt"  
                width="150px"
            >
            </el-table-column>
            <el-table-column
                label="" 
                width="120px"
                align="right"
            >
                <template slot-scope="props">
                    <span class="tr-oper-delete" @click.stop="handleDeleteTask(props.row)"><i class="mouse-over el-icon-delete"></i></span>
                </template>
            </el-table-column>
        </el-table>
    </div>

    <SetTaskDialog
        :visible.sync="setTaskDialogVisiblity"
        :method="setTaskMethod"
        :configList="extConfigList"
        @confirm="handleConfirm"
    >
    </SetTaskDialog>

</tr-dashboard>

</template>
<script>

import path from 'path';
import { mapState } from 'vuex';
import moment from 'moment';

import SetTaskDialog from './SetTaskDialog';

import { getExtensionConfigs, ifProcessRunning } from '__gUtils/busiUtils';
import { deleteProcess } from '__gUtils/processUtils';
import { TASK_EXTENSION_DIR } from '__gConfig/pathConfig';
import { switchTask } from '__io/actions/base';


import baseMixin from '@/assets/js/mixins/baseMixin';

export default {

    name: 'task',
    
    mixins: [ baseMixin ],

    components: {
        SetTaskDialog
    },

    data () {

        return {
            setTaskMethod: 'add',
            setTaskDialogVisiblity: false,
            searchFilterKey: 'processId',
            extConfigList: [],
        }
    },

    mounted () {
       this.getExtensionConfigs()
    },

    computed: {
        ...mapState({
            processStatus: state => state.BASE.processStatus,
            processStatusWithDetail: state => state.BASE.processStatusWithDetail
        }),
    },

    watch: {
        processStatusWithDetail (newProcess) {
            this.tableList = Object.keys(newProcess || {})
                .map(key => {
                    return {
                        processId: key,
                        processStatus: newProcess[key].status,
                        createdAt: moment(newProcess[key].created_at).format('YYYY-MM-DD HH:mm:ss'),
                        ...newProcess[key]
                    }
                })
                .filter(({ processId }) => {
                    if (processId.includes('task')) {
                        return true
                    }
                    return false
                })
        },

    },

    
    methods: {

        handleAddTask () {

            if (!this.extConfigList.length) {
                this.$message.warning('暂无交易任务可选项！')
                return;
            }

            this.setTaskMethod = 'add';
            this.setTaskDialogVisiblity = true;
        },

        handleConfirm (extSettingData, configIndex) {
            const configInfo = this.extConfigList[configIndex];
            const currentTimestamp = moment().format('HHmmss') + '';
            const processName = 'task_' + configInfo.key + '_' + currentTimestamp;
            const packageJSONPath = configInfo.packageJSONPath;
            return switchTask(processName, true, {
                args: `-c ${extSettingData}`,
                cwd: path.resolve(packageJSONPath, '..', 'lib'),
                script: 'index.js'
            })
            
        },

        handleDeleteTask (data) {
            const { processId } = data;
            return this.$confirm(`确认停止并删除交易任务 ${processId} 吗？`, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => deleteProcess(processId))
            .then(() => this.$message.success('操作成功！'))
            .catch((err) => {
                if(err == 'cancel') return
                this.$message.error(err.message || '操作失败！')
            })
        },

        handleTaskSwitch (e, data) {
            const { processId, args, cwd, script } = data;
            return switchTask(processId, e,  {
                args,
                cwd,
                script
            })
        },

        getExtensionConfigs () {
            return getExtensionConfigs(TASK_EXTENSION_DIR)
                .then(exts => {
                    this.extConfigList = exts.filter(({ type }) => type === 'task')
                })
        },
    }

}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';


</style>
