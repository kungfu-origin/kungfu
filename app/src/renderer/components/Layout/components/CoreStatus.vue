<template>
    <el-popover
        placement="top-start"
        width="auto"
        trigger="click"
        popper-class="kf-footer-popover"
        >
        <div class="core-status-content">
            <div class="core-item" >
                <div class="core-status">
                    <span class="core-process-item core-process-title text-overflow" title="主进程">
                        主控进程  <el-tag type="warning">Master</el-tag>
                    </span>
                    <span class="core-process-item text-overflow" style="width: 71px;">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('master', processStatus)"
                        :value="buildState('master')"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="core-process-item switch"></span>
                    <span class="core-process-item get-log">
                        <i class="el-icon-document mouse-over" title="打开日志文件" @click="handleOpenLog('master.log')" ></i>
                    </span>
                </div>
            </div>
            <div class="core-item" >
                <div class="core-status">
                    <span class="core-process-item  core-process-title text-overflow" title="数据进程">
                        数据进程 <el-tag>Ledger</el-tag>
                    </span>
                    <span  class="core-process-item text-overflow" style="width: 71px;">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('ledger', processStatus)"
                        :value="buildState('ledger')"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="core-process-item switch">
                        <el-switch :value="$utils.ifProcessRunning('ledger', processStatus)" @change="handleLedgerSwitch"></el-switch>
                    </span>
                     <span class="core-process-item get-log">
                        <i class="el-icon-document mouse-over" title="打开日志文件" @click="handleOpenLog('ledger.log')" ></i>
                    </span>
                </div>
            </div>
        </div>
        <span slot="reference" :class="[currentStatus]">
            <i class="el-icon-monitor"></i>
            主进程状态
        </span>
    </el-popover>
</template>
<script>
import { mapGetters, mapState } from 'vuex';
import { statusConfig } from '__gConfig/statusConfig';
import { ifProcessRunning } from '__gUtils/busiUtils';
import { switchLedger } from '__io/actions/base';
import { LOG_DIR } from '__gConfig/pathConfig';

const path = require('path');

export default {
    data(){
        let statusLevel = {};
        Object.keys(statusConfig || {}).map(key => {
            statusLevel[key] = statusConfig[key].level;
        })
        this.nasterErrController = false;
        this.ledgerErrController = false;
        return {
            statusLevel
        }
    },

    computed: {

        ...mapState({
            tdList: state => state.ACCOUNT.tdList,
            mdTdState: state => state.ACCOUNT.mdTdState,
            processStatus: state => state.BASE.processStatus
        }),

        currentStatus(){
            const t = this;
            if(t.processStatus === null){
                return 'color-gray'
            }

            if(!ifProcessRunning('master', t.processStatus)){
                if(!t.nasterErrController && !!t.processStatus['master']){
                    t.$message.error('主控进程断开，不可交易，请重启应用！', 0)
                    t.nasterErrController = true;  
                }
                return 'color-red'
            }

            if(!ifProcessRunning('ledger', t.processStatus)){
                if(!t.ledgerErrController && !!t.processStatus['ledger']){
                    t.$message.error('数据进程断开，交易数据将会丢失，请重启应用！', 0)
                    t.ledgerErrController = true;  
                }
                return 'color-red'
            }

            return 'color-green'
        }
    },

    methods: {
        buildState(processId) {
            const t = this;
            return t.processStatus[processId]
        },

        handleLedgerSwitch (e) {
            switchLedger(e)
        },

        handleOpenLog(target) {
            this.$showLog(path.join(LOG_DIR, target))
        }
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
.core-status-popover{
    box-shadow: 0px 0px 30px $bg;
}

.core-status-content{
    font-family: Consolas, Monaco, monospace,"Microsoft YaHei",sans-serif;
    
    .core-item{
        width: 330px;
        margin: 10px;

        .core-status{
            display: flex;
            font-size: 14px;

            .core-process-item{
                display: inline-block;
                padding-left: 5px;
                color: $font_5;
                text-align: left;
                vertical-align: bottom;
                color: $font;

                &.get-log {
                    width: 20px;
                }

                &.switch {
                    flex: 1;
                }

                &.core-process-title{
                    width: 155px;
                }

                &.status-switch{
                    width: 40px;
                }
            }  
        }
    }
}


</style>
