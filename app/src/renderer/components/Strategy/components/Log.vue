<template>
<tr-dashboard title="交易日志">
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
            <i class="el-icon-document mouse-over" title="打开日志文件"  @click="handleOpenLogFile(logPath)"></i>
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
import { mapState } from 'vuex'
import moment from 'moment'
import { debounce, throttle, throttleInsert, dealLogMessage } from '__gUtils/busiUtils'
import { buildProcessLogPath } from '__gConfig/pathConfig';
import { Tail } from 'tail';
import { clearFileContent, addFile, openReadFile, existsSync } from '__gUtils/fileUtils';
import { ipcRenderer } from 'electron';
import { platform } from '__gConfig/platformConfig';
const BrowserWindow = require('electron').remote.BrowserWindow;


export default {
    name: 'log',
    data() {
        this.schema = [
            {
                label: '时间',
                prop: 'updateTime',
                width: '230px'
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
        return {
            rendererTable: false,
            tableData: Object.freeze([]),
            ifScrollToBottom: false,
            searchKeyword: '',
            logCount: 10000,
        }
    },

    computed:{
        ...mapState({
            processId: state => state.STRATEGY.currentStrategy.strategy_id
        }),

        logPath(){
            const t = this;
            return buildProcessLogPath(t.processId);
        },
    },

    watch: {
        searchKeyword: debounce(function(newVal){
            const t = this;
            t.resetData(true)
            t.processId && t.init(t.processId, t.logPath, t.searchKeyword)
        }),

        processId(val){
            const t = this;
            t.resetData();
            if(!val) return;
            t.rendererTable = false;
            t.$nextTick().then(() => {
                t.rendererTable = true;
                t.init(t.processId, t.logPath)
            })
        }
    },

    mounted(){
        const t = this;
        t.rendererTable = true;
        t.resetData();
        t.processId && t.init(t.processId, t.logPath);
    },

    destroyed(){
        this.resetData()
        ipcRenderer.removeAllListeners('res-strategy-log')
    },

    methods:{
        //清空
        handleClearLog(){
            const t = this;
            t.$confirm('确认清空该日志？', '提示', {
                confirmButtonText: '确 定',
                cancelButtonText: '取 消',
            })
            .then(() => clearFileContent(buildProcessLogPath(t.processId)))
            .then(() => {
                t.resetData();
                t.processId && t.init(t.processId, t.logPath)
                t.$message.success('操作成功！')
            })
            .catch((err) => {
                if(err == 'cancel') return
                t.$message.error(err.message || '操作失败！')
            })
        },

        handleOpenLogFile(logPath){
            openReadFile(logPath);
        },

        handleRefresh(){
            const t = this;
            t.resetData();
            t.processId && t.init(t.processId, t.logPath, t.searchKeyword)
        },

        init: debounce(function(processId, logPath, searchKeyword){
            const t = this;
            //文件不存在则创建
            if(!existsSync(logPath)){
                t.tableData = Object.freeze([])
                addFile('', logPath, 'file')
            }

            t.getLogByTask(logPath, searchKeyword).then(logList => {
                t.tableData = Object.freeze(logList)
                t.scrollToBottom()
            }).catch(err => {
                t.tableData = Object.freeze([])
            }).finally(() => {
                t.startWatchingTail(processId, logPath, searchKeyword)
            })
        }),

        getLogByTask(logPath, searchKeyword){
            const t = this;
            return new Promise((resolve, reject) => {
                t.$utils.buildTask('getStrategyLog', BrowserWindow.getFocusedWindow(), BrowserWindow).then(({win, curWinId}) => {
                    win.webContents.send('get-strategy-log', {
                        winId: curWinId,
                        logPath,
                        searchKeyword
                    });
                    ipcRenderer.on('res-strategy-log', function resLog(event, logs){
                        resolve(Object.freeze(logs))
                        ipcRenderer.removeAllListeners('res-strategy-log')
                    })
                }).catch(err => reject(err))
            })
        },


        //开始监听日志尾部
        startWatchingTail(processId, logPath, searchKeyword){
            const t = this;
            let timer = null;
            let logWaitList = [];
            let throttleInsertLog = throttleInsert(500)
            let throttleClearLog = throttle(() => {
                    const len = t.tableData.length
                    if(len > 1000) t.tableData = t.tableData.slice(len - 1000, len)
                }, 60000);
                
            t.tailObserver = new Tail(logPath, {
                flushAtEOF: true,
                useWatchFile: platform === 'mac',
                follow: true,
            });   
            t.tailObserver.watch();    
            t.tailObserver.on('line', line => ((curProcId, curKw) => {
                if(curKw) return;
                if(curProcId !== processId) return;
                const logData = dealLogMessage(line, t.searchKeyword);
                throttleInsertLog(logData).then(logList => {
                    if(!logList) return;
                    t.tableData = t.pushTableData(logList);
                })
                throttleClearLog()
                if(t.ifScrollToBottom) t.scrollToBottom()
            })(processId, searchKeyword))

            t.tailObserver.on('error', err => {
                if(t.tailObserver !== null) t.clearTailWatcher();
                t.tailObserver = null;
            }) 
        },
        
        //往日志列表里推送数据
        pushTableData(itemList){
            const t = this;
            const tableData = t.tableData.slice(0);
            itemList.forEach(item => {
                t.logCount++;
                if(!item || !item.message) return;
                tableData.push(Object.freeze({
                    ...item,
                    id: t.logCount
                }))
            }) 
            return Object.freeze(tableData)
        },

        //重置数据
        resetData(ifSearchKeyword=false) {
            const t = this;
            t.logCount = 10000;
            !ifSearchKeyword && (t.searchKeyword = '');
            t.clearTailWatcher();
            t.tableData = Object.freeze([]);
            t.ifScrollToBottom = false;
        },

        clearTailWatcher(){
            const t = this;
            if(t.tailObserver != null) t.tailObserver.unwatch();
            t.tailObserver = null;
            return true;
        },

        //加载完数据
        scrollToBottom: throttle(function() {
            const t = this;
            if(!t.$refs['logTable']) return;
            const $logTable = t.$refs['logTable'];
            if(!$logTable) return;
            t.$nextTick().then(() => { 
                $logTable.triggerToBottom()
            })
        }, 1000),

        renderCellClass(prop, item){
            const t = this;
            if(item.type === 'error'){
                return 'red'
            }else{
                return ''
            }
        },
    }
}
</script>

<style lang="scss">

</style>