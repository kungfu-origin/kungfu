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
                width="300px"  
                >
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
                label="执行时间"
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
        :configList="extConfigList"
        @confirm="handleConfirm"
    >
    </SetTaskDialog>

</tr-dashboard>

</template>
<script>

import path from 'path';
import moment from 'moment';
import minimist from 'minimist';
import { mapState } from 'vuex';

import SetTaskDialog from './SetTaskDialog';

import { getExtensionConfigs, findTargetFromArray, ifProcessRunning } from '__gUtils/busiUtils';
import { deleteProcess } from '__gUtils/processUtils';
import { removeFileFolder } from '__gUtils/fileUtils';
import { TASK_EXTENSION_DIR, buildProcessLogPath } from '__gConfig/pathConfig';
import { switchTask } from '__io/actions/base';

import baseMixin from '@/assets/js/mixins/baseMixin';
import openLogMixin from '@/assets/js/mixins/openLogMixin';

export default {

    name: 'task',
    
    mixins: [ baseMixin, openLogMixin ],

    components: {
        SetTaskDialog
    },

    data () {
        this.ifProcessRunning = ifProcessRunning;
        
        return {
            setTaskMethod: 'add',
            setTaskInitData: {},
            setTaskInitKey: '',
            setTaskTarget: null,
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
                        createdAt: newProcess[key].created_at ? moment(newProcess[key].created_at).format('YYYY-MM-DD HH:mm:ss') : '--',
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
            this.setTaskInitData = {};
            this.setTaskInitKey = ""
            this.setTaskTarget = null
            this.setTaskDialogVisiblity = true;
        },

        handleOpenUpdateTaskDialog (data) {
            this.setTaskMethod = 'update';
            //TODO 写活
            const minimistConfig = this.getMinimistConfig()
            this.setTaskInitData = minimist(data.args, minimistConfig)
            this.setTaskInitKey = this.getTaskConfigKeyFromProcessId(data.processId)
            this.setTaskTarget = data;
            this.setTaskDialogVisiblity = true;
        },

        handleConfirm (extSettingJSONString, configKey) {
            const configInfo = this.getTargetConfigByKey(configKey);
            if (!configInfo) {
                this.$message.error('配置信息不存在！')
                return
            }
            const extSettingData = JSON.parse(extSettingJSONString)
            const { key, uniKey, packageJSONPath } = configInfo;
            const processNameByUniKey = this.formUnikeyInProcessName(uniKey, extSettingData);
            const processName = 'task_' + key + '_' + processNameByUniKey;
            const args = this.formArgs(extSettingData);

            return this.preUpdate()
                .then(res => {
                    if (!res) return Promise.resolve(true)
                    return switchTask(true, {
                        name: processName,
                        args,
                        cwd: process.env.NODE_ENV === 'production' ? path.resolve(packageJSONPath, '..') : path.resolve(packageJSONPath, '..', 'lib'),
                    })
                })
        },

        handleDeleteTask (data, update = false) {
            const { processId } = data;
            const tips = update 
                ? `更新配置需停止并删除交易任务 ${processId}，确认停止并删除吗？`
                : `确认停止并删除交易任务 ${processId} 吗？`

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

        handleTaskSwitch (e, data) {
            const { processId, args, cwd } = data;
            return switchTask(e, {
                name: processId,
                args: args.join(' '),
                cwd
            })
        },

        getMinimistConfig () {
            let minimistConfig = {
                string: [],
                boolean: [],
            };
            this.extConfigList.forEach(config => {
                const c = config.config;
                c.forEach(item => {
                    const { key, type } = item;
                    if (type === 'instrumentId') {
                        if (!minimistConfig.string.includes(key)) {
                            minimistConfig.string.push(key)                        
                        }
                    }

                    if (type === 'bool') {
                        if (!minimistConfig.boolean.includes(key)) {
                            minimistConfig.boolean.push(key)
                        }
                    }
                })
            })

            return minimistConfig
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
            return findTargetFromArray(this.extConfigList, 'key', key)
        },

        formArgs (data) {
            return Object.keys(data || {})
                .map(key => `--${key} ${data[key]}`)
                .join(' ')
        },

        getTaskConfigKeyFromProcessId (processId) {
            let processIdSplit = processId.split('_');
            return processIdSplit[1]
        },

        getExtensionConfigs () {
            return getExtensionConfigs(TASK_EXTENSION_DIR)
                .then(exts => {
                    this.extConfigList = Object.freeze(exts.filter(({ type }) => type === 'task'))
                })
        },
    }

}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';


</style>
