<template>
<tr-dashboard :title="noTitle ? '' : '算法任务'">
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
        @row-click="handleRowClick"
        :row-class-name="handleSelectRow"
        >
            <el-table-column
                prop="processId"
                label="任务ID"
                sortable
                :show-overflow-tooltip="true"       
                min-width="270px"
                >
            </el-table-column>
            <el-table-column
                prop="subType"
                label="类型"
                sortable
                :show-overflow-tooltip="true"       
                >
                <template slot-scope="props">
                    <el-tag
                        :type="(TaskTypeConfig[props.row.subType] || {}).color || ''" 
                        >
                            {{ (TaskTypeConfig[props.row.subType] || {}).name || props.row.subType }}
                        </el-tag>                
                    </template>
            </el-table-column>
            <el-table-column
                label="状态"
                sortable  
                show-overflow-tooltip
            >
                <template slot-scope="props">
                    <tr-status 
                    v-if="ifProcessRunning(props.row.processId, processStatus)"
                    :value="props.row.processStatus"
                    ></tr-status>
                    <tr-status v-else></tr-status>
                </template>
            </el-table-column>
            <el-table-column
                label="执行时间"
                sortable  
                prop="createdAt"  
                min-width="180px"
            >
            </el-table-column>
            <el-table-column
                label="进程"
                sortable    
            >
                <template slot-scope="props">
                    <el-switch
                    :value="ifProcessRunning(props.row.processId, processStatus)"
                    @change="e => handleTaskSwitch(e, props.row)"
                    ></el-switch> 
                </template>
            </el-table-column>
            <el-table-column
                label="" 
                align="right"
                width="100px"
            >
                <template slot-scope="props">
                    <span class="tr-oper" @click.stop="handleOpenLogFile(props.row.processId)"><i class="el-icon-document mouse-over" title="打开日志文件"></i></span>
                    <span class="tr-oper" @click.stop="handleOpenUpdateTaskDialog(props.row)"><i class="el-icon-setting mouse-over" title="设置"></i></span>
                    <span class="tr-oper-delete" @click.stop="handleDeleteTask(props.row)"><i class="mouse-over el-icon-delete"></i></span>
                </template>
            </el-table-column>
        </el-table>
    </div>

    <SetTaskDialog
        v-if="setTaskDialogVisiblity"
        :visible.sync="setTaskDialogVisiblity"
        :value="setTaskInitData"
        :currentActiveConfigKey="setTaskInitKey"
        :method="setTaskMethod"
        :configList="taskExtConfigList"
        :outsideAddTaskType="outsideAddTaskType"
        :outsideAddTaskInitData="outsideAddTaskInitData"
        @confirm="handleConfirm"
    >
    </SetTaskDialog>

</tr-dashboard>

</template>
<script>

import path from 'path';
import moment from 'moment';
import minimist from 'minimist';
import { mapGetters, mapState } from 'vuex';

import SetTaskDialog from './SetTaskDialog';

import { findTargetFromArray, ifProcessRunning } from '__gUtils/busiUtils';
import { deleteProcess, buildProcessStatusWidthDetail } from '__gUtils/processUtils';
import { removeFileFolder } from '__gUtils/fileUtils';
import { buildProcessLogPath } from '__gConfig/pathConfig';
import { switchTask } from '__io/actions/base';
import { TaskTypeConfig } from '@kungfu-trader/kungfu-shared/config/tradingConfig';

import baseMixin from '@/assets/mixins/baseMixin';
import openLogMixin from '@/assets/mixins/openLogMixin';

export default {

    name: 'task',
    
    mixins: [ baseMixin, openLogMixin ],

    props: {

        noTitle: {
            type: Boolean,
            default: true,
        },
    },

    components: {
        SetTaskDialog
    },

    data () {
        this.ifProcessRunning = ifProcessRunning;
        this.TaskTypeConfig = TaskTypeConfig;

        return {
            setTaskMethod: 'add',
            setTaskInitData: {},
            setTaskInitKey: '',
            setTaskTarget: null,
            setTaskDialogVisiblity: false,
            searchFilterKey: 'processId',
            
            //从外部传入，通过 bus.$emit
            outsideAddTaskType: '',
            outsideAddTaskInitData: {}
            
            
        }
    },

    mounted () {
        this.$bus.$on('set-task', ({ type, initData }) => {
            this.handleAddTaskFromOutSide(type, initData)
        })
    },

    beforeDestroy () {
        this.$bus.$off('set-task');
    },

    computed: {
        ...mapState({
            taskExtConfigList: state => state.BASE.taskExtConfigList,
            processStatus: state => state.BASE.processStatus,
            processStatusWithDetail: state => state.BASE.processStatusWithDetail,
            currentTask: state => state.BASE.currentTask,
            currentTaskId: state => (state.BASE.currentTask).name
        }),

        ...mapGetters([
            "taskExtMinimistConfig"
        ])
    },

    watch: {
        processStatusWithDetail (newProcess) {

            this.tableList = Object.keys(newProcess || {})
                .map(key => {
                    const targetProcess = newProcess[key];
                    return this.buildTaskProcessItem(key, targetProcess)
                })
                .filter(({ processId }) => {
                    if (processId.includes('task')) {
                        return true
                    }
                    return false
                })

            if (!this.tableList.length) {
                this.$store.dispatch('setCurrentTask', {})
            } else if (!this.currentTaskId) {
                this.$store.dispatch('setCurrentTask', this.tableList[0])
            }
        },
    },

    methods: {

        //选中行的背景颜色
        handleSelectRow(row) {
            if(row.row.name == this.currentTaskId) {
                return 'selected-bg'
            }
        },

        handleRowClick (row) {
            this.$store.dispatch('setCurrentTask', row)
        },

        handleAddTaskFromOutSide (type, initData) {
            this.outsideAddTaskType = type;
            this.outsideAddTaskInitData = initData;
            this.setTaskMethod = 'add';
            this.setTaskInitData = {};
            this.setTaskInitKey = "";
            this.setTaskTarget = null;
            this.setTaskDialogVisiblity = true;
        },

        handleAddTask () {
            if (!this.taskExtConfigList.length) {
                this.$message.warning('暂无算法任务可选项！')
                return;
            }

            this.outsideAddTaskType = '';
            this.outsideAddTaskInitData = {};
            this.setTaskMethod = 'add';
            this.setTaskInitData = {};
            this.setTaskInitKey = "";
            this.setTaskTarget = null;
            this.setTaskDialogVisiblity = true;
        },

        handleOpenUpdateTaskDialog (data) {
            this.outsideAddTaskType = '';
            this.outsideAddTaskInitData = {};
            this.setTaskMethod = 'update';
            this.setTaskInitData = minimist(data.args, this.taskExtMinimistConfig)
            this.setTaskInitKey = this.setTaskInitData.configKey || '';
            this.setTaskTarget = data;
            this.setTaskDialogVisiblity = true;
        },

        handleConfirm (extSettingJSONString, configKey) {
            const configInfo = this.getTargetConfigByKey(configKey);
            if (!configInfo) {
                this.$message.error('配置信息不存在！')
                return
            }
            const extSettingData = JSON.parse(extSettingJSONString);

            const { key, uniKey, packageJSONPath, subType } = configInfo;
            const processNameByUniKey = this.formUnikeyInProcessName(uniKey, extSettingData);
            const processName = 'task_' + key + '_' + processNameByUniKey;
            const args = this.formArgs(extSettingData);
            const cwd = process.env.NODE_ENV === 'production' ? path.resolve(packageJSONPath, '..') : path.resolve(packageJSONPath, '..', 'lib');

            return this.preUpdate()
                .then(res => {
                    if (!res) return Promise.resolve(true)
                    return switchTask(true, {
                        name: processName,
                        args,
                        cwd
                    })
                    .then(apps => {
                        const resolvedApps = buildProcessStatusWidthDetail(apps);
                        //相当于模拟一个pmDetailData(通过pm2 start 返回的appdata跟pm2.list返回的不一致)
                        const processWithDetail = { 
                            ...Object.values(resolvedApps)[0] || {},
                            args: args.split(' '),
                            cwd,
                            name: processName,
                            subType,
                            configKey
                        };

                        this.$store.dispatch('setCurrentTask', this.buildTaskProcessItem(processName, processWithDetail))
                    })
                })
        },

        handleDeleteTask (data, update = false) {
            const { processId } = data;
            const tips = update 
                ? `更新配置需停止并删除算法任务 ${processId}，确认停止并删除吗？`
                : `确认停止并删除算法任务 ${processId} 吗？`

            return this.$confirm(tips, '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => deleteProcess(processId))
            .then(() => removeFileFolder(buildProcessLogPath(processId)))
            .then(() => this.$message.success('操作成功！'))
            .then(() => true)
            .catch((err) => {
                if(err !== 'cancel') {
                    this.$message.error(err.message || '操作失败！')
                }
                return false
            })
        },

        buildTaskProcessItem (key, pmData) {
            const argsConfig = minimist(pmData.args, this.taskExtMinimistConfig)

            return {
                processId: key,
                processStatus: pmData.status,
                createdAt: pmData.created_at ? moment(pmData.created_at).format('YYYY-MM-DD HH:mm:ss') : '--',
                configKey: argsConfig.configKey,
                subType: argsConfig.subType,
                ...pmData
            }
        },

        handleTaskSwitch (e, data) {
            const { processId, args, cwd } = data;
            console.log('switch task: ',processId, args, cwd)
            return switchTask(e, {
                name: processId,
                args: args.join(' '),
                cwd
            })
        },

        formUnikeyInProcessName (uniKey, form) {
            if (typeof uniKey === 'string') {
                return form[uniKey]
            } else {
                return uniKey.map(key => form[key]).join('_')
            }
        },

        preUpdate () {
            if (this.setTaskTarget) {
                return this.handleDeleteTask(this.setTaskTarget, true)
            }

            return Promise.resolve(true)
        },

        getTargetConfigByKey (key) {
            return findTargetFromArray(this.taskExtConfigList, 'key', key)
        },

        formArgs (data) {
            return Object.keys(data || {})
                .map(key => `--${key} ${data[key]}`)
                .join(' ')
        },
    }

}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';


</style>
