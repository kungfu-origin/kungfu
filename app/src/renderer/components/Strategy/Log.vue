<template>
<tr-dashboard :title="name ? name : `交易日志 ${currentTitle}`">
    <div slot="dashboard-header">
        <tr-dashboard-header-item>
            <el-checkbox v-model="ifScrollToBottom">跟踪至底部</el-checkbox>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <tr-search-input v-model.trim="searchKeyword"></tr-search-input>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-refresh mouse-over" title="刷新" @click="handleRefresh"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <i class="el-icon-document mouse-over" title="打开日志文件"  @click="handleOpenLogFile(logPath, true)"></i>
        </tr-dashboard-header-item>
        <tr-dashboard-header-item>
            <el-button size="mini" @click="handleClearLog" title="清空">清空</el-button>
        </tr-dashboard-header-item>
    </div>
        <tr-table
        v-if="rendererTable"
        scrollerType="dynamic"
        ref="logTable"
        :data="tableData"
        :schema="schema"
        :sizeDependencies="['message']"
        :renderCellClass="renderCellClass"
        ></tr-table>
</tr-dashboard>
     
</template>

<script>
import fse from 'fs-extra'; 
import { mapState } from 'vuex'
import { debounce, throttle, throttleInsert, dealLogMessage } from '__gUtils/busiUtils'
import { buildProcessLogPath } from '__gConfig/pathConfig';
import { Tail } from 'tail';
import { writeFile, addFileSync } from '__gUtils/fileUtils';
import { ipcRenderer } from 'electron';

import Workers from '@/workers/index';

import openLogMixin from '@/assets/mixins/openLogMixin';

export default {
    name: 'log',

    mixins: [ openLogMixin ],

    props: {
        name: {
            type: String,
            default: ''
        }
    },

    data() {
        this.logColor = {
            info: 'green',
            trace: '',
            error: 'red',
            warning: 'yellow',
            debug: 'blue',
            critical: 'red'
        }

        this.schema = [
            {
                label: '时间',
                prop: 'updateTime',
                width: '200px'
            },
            {
                label: '类型',
                prop: 'type',
                width: '60px'
            },
            
            {
                label: '信息详情',
                prop: 'message',
            },
        ];
        this.tailObserver = null;
        this.logMaxLenLimit = 500;
        return {
            rendererTable: false,
            tableData: Object.freeze([]),
            ifScrollToBottom: true,
            searchKeyword: '',
            logCount: 10000,
        }
    },

    computed: {
        ...mapState({
            processId: state => state.STRATEGY.currentStrategy.strategy_id
        }),

        currentTitle () {
            return this.processId ? `${this.processId}` : ''
        },

        logPath () {
            return buildProcessLogPath(this.processId);
        },
    },

    watch: {
        searchKeyword: debounce(function () {
            this.resetData(true)
            this.processId && this.init(this.logPath, this.searchKeyword)
        }),

        processId: debounce(function (val) {
            this.resetData();
            if(!val) return;
            this.rendererTable = false;
            this.$nextTick().then(() => {
                this.rendererTable = true;
                this.init(this.logPath, this.searchKeyword)
            })
        }, 100),

        ifScrollToBottom(val){
            if(val) this.scrollToBottom()
        }
    },

    mounted () {
        this.rendererTable = true;
        this.resetData();
        this.processId && this.init(this.logPath, this.searchKeyword)

        this.bindGetLogWorkerListener();
    },

    destroyed () {
        this.resetData()
        ipcRenderer.removeAllListeners('res-strategy-log')
    },

    methods:{
        //清空
        handleClearLog () {
            this.$confirm('确认清空该日志？', '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => writeFile(buildProcessLogPath(this.processId), ''))
            .then(() => {
                this.resetData();
                this.processId && this.init(this.logPath, this.searchKeyword)
                this.$message.success('操作成功！')
            })
            .catch((err) => {
                if(err == 'cancel') return
                this.$message.error(err.message || '操作失败！')
            })
        },

        handleRefresh () {
            this.resetData();
            this.processId && this.init(this.logPath, this.searchKeyword)
        },

        bindGetLogWorkerListener () {
            const self = this;
            Workers.getLogWorker.onmessage = (event) => {
                const {
                    list,
                    logPath,
                    searchKeyword
                } = event.data || [];

                if (logPath !== this.logPath || searchKeyword !== this.searchKeyword) {
                    return;
                }

                self.tableData = Object.freeze(list || [])
                self.scrollToBottom();
                self.startWatchingTail(self.processId, logPath, searchKeyword)
            }
        },

        init: debounce(function (logPath, searchKeyword) {
            //文件不存在则创建
            if(!fse.existsSync(logPath)){
                this.tableData = Object.freeze([])
                addFileSync('', logPath, 'file')
            }

            Workers.getLogWorker.postMessage({
                logPath,
                searchKeyword
            })
        }, 100),

        //开始监听日志尾部
        startWatchingTail (processId, logPath, searchKeyword) {
            this.clearTailWatcher();
            let throttleInsertLog = throttleInsert(500)
            let throttleClearLog = throttle(() => {
                    const len = this.tableData.length
                    if(len > 1000) this.tableData = this.tableData.slice(len - 1000, len)
                }, 60000);
            
            this.tailObserver = new Tail(logPath, {
                flushAtEOF: true,
                useWatchFile: true,
                follow: true,
            });   
            this.tailObserver.watch();  
            this.tailObserver.on('line', line => ((curProcId, curKw) => {
                if(curKw) return;
                if(curProcId !== this.processId) return;
                const logData = dealLogMessage(line, this.searchKeyword);
                throttleInsertLog(logData).then(logList => {
                    if(!logList) return;
                    this.tableData = this.pushTableData(logList);
                    if(this.ifScrollToBottom) this.scrollToBottom();
                })
                throttleClearLog()
            })(processId, searchKeyword))

            this.tailObserver.on('error', err => {
                if(this.tailObserver !== null) this.clearTailWatcher();
                this.tailObserver = null;
            }) 
        },
        
        //往日志列表里推送数据
        pushTableData (itemList) {
            let tableData = this.tableData.slice(0);
            itemList.kfForEach(item => {
                this.logCount++;
                if(!item || !item.message) return;
                tableData.push(Object.freeze({
                    ...item,
                    nano: true,
                    id: this.logCount
                }))
            }) 

            //最大log数
            const len = tableData.length;
            if (len > this.logMaxLenLimit) {
                tableData = tableData.slice(len - 500, len)
            }

            return Object.freeze(tableData)
        },

        //重置数据
        resetData (ifSearchKeyword=false) {
            this.logCount = 10000;
            !ifSearchKeyword && (this.searchKeyword = '');
            this.clearTailWatcher();
            this.tableData = Object.freeze([]);
            this.ifScrollToBottom = true;
        },

        clearTailWatcher () {
            if(this.tailObserver != null) this.tailObserver.unwatch();
            this.tailObserver = null;
            return true;
        },

        //加载完数据
        scrollToBottom: throttle(function() {
            const $logTable = this.$refs['logTable'];
            if(!$logTable) return
            this.$nextTick()
                .then(() => { 
                    $logTable.triggerToBottom()
                })
        }, 1000),

        renderCellClass (prop, item) {
            if(prop === 'type') {
                return this.logColor[item.type] || ''
            }
            if(item.type === 'error' || item.type === 'critical') {
                return this.logColor[item.type]
            }
            
        },
    }
}
</script>

<style lang="scss">

</style>