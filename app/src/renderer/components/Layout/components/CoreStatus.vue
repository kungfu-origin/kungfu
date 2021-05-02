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
                        v-if="ifProcessRunning('master', processStatus)"
                        :value="buildState('master')"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="core-process-item switch" v-if="NODE_ENV === 'development'"></span>
                    <span class="core-process-item get-log">
                        <i class="el-icon-document mouse-over" title="打开日志文件" @click="handleOpenLogFile('master')" ></i>
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
                        v-if="ifProcessRunning('ledger', processStatus)"
                        :value="buildState('ledger')"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="core-process-item switch" v-if="NODE_ENV === 'development'">
                        <el-switch  :value="ifProcessRunning('ledger', processStatus)" @change="handleLedgerSwitch"></el-switch>
                    </span>
                     <span class="core-process-item get-log">
                        <i class="el-icon-document mouse-over" title="打开日志文件" @click="handleOpenLogFile('ledger')" ></i>
                    </span>
                </div>
            </div>
            <div class="core-item" >
                <div class="core-status">
                    <span class="core-process-item  core-process-title text-overflow" title="数据进程">
                        守护进程 <el-tag>Deamon</el-tag>
                    </span>
                    <span  class="core-process-item text-overflow" style="width: 71px;">
                        <tr-status 
                        v-if="ifProcessRunning('kungfuDeamon', processStatus)"
                        :value="buildState('kungfuDeamon')"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="core-process-item switch" v-if="NODE_ENV === 'development'"></span>
                     <span class="core-process-item get-log">
                        <i class="el-icon-document mouse-over" title="打开日志文件" @click="handleOpenLogFile('kungfuDeamon')" ></i>
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
import { mapState } from 'vuex';
import { statusConfig } from '__gConfig/statusConfig';
import { ifProcessRunning } from '__gUtils/busiUtils';
import { switchLedger } from '__io/actions/base';

import openLogMixin from '@/assets/mixins/openLogMixin';

export default {
    mixins: [ openLogMixin ],

    data () {
        let statusLevel = {};
        Object.keys(statusConfig || {}).map(key => {
            statusLevel[key] = statusConfig[key].level;
        })
        this.nasterErrController = false;
        this.ledgerErrController = false;
        this.ifProcessRunning = ifProcessRunning;
        return {
            statusLevel,
            NODE_ENV: process.env.NODE_ENV
        }
    },

    computed: {

        ...mapState({
            tdList: state => state.ACCOUNT.tdList,
            mdTdState: state => state.ACCOUNT.mdTdState,
            processStatus: state => state.BASE.processStatus
        }),

        currentStatus(){
            if(this.processStatus === null){
                return 'color-gray'
            }

            if(!ifProcessRunning('master', this.processStatus)){
                if(!this.nasterErrController && !!this.processStatus['master']){
                    this.$message.error('主控进程断开，不可交易，请重启应用！', 0)
                    this.nasterErrController = true;  
                }
                return 'color-red'
            }

            if(!ifProcessRunning('ledger', this.processStatus)){
                if(!this.ledgerErrController && !!this.processStatus['ledger']){
                    this.$message.error('数据进程断开，交易数据将会丢失，请重启数据进程！', 0)
                    this.ledgerErrController = true;  
                }
                return 'color-red'
            }

            return 'color-green'
        }
    },

    methods: {
        buildState(processId) {
            return this.processStatus[processId]
        },

        handleLedgerSwitch (e) {
            switchLedger(e)
        },
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
                    margin-right: 20px;
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
