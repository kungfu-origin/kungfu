<template>
    <el-popover
        placement="top-start"
        width="auto"
        trigger="click"
        popper-class="core-status-popover"
        >
        <div class="core-status-content">
            <div class="core-item" >
                <div class="core-status">
                    <span class="core-process-item core-process-title text-overflow" title="主进程">
                        主进程  <el-tag type="danger">master</el-tag>
                    </span>
                    <span  class="core-process-item text-overflow" style="width: 81px;">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('master', processStatus)"
                        :value="buildState('master')"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <!-- <span class="core-process-item status-switch" @click.stop>
                        <el-switch :value="$utils.ifProcessRunning('md_' + accountItem.source_name, processStatus)" @change="handleMdSwitch($event, accountItem)"></el-switch>
                    </span> -->
                </div>
            </div>
            <div class="core-item" >
                <div class="core-status">
                    <span class="core-process-item  core-process-title text-overflow" title="数据进程">
                        数据进程
                    </span>
                    <span  class="core-process-item text-overflow" style="width: 81px;">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('watcher', processStatus)"
                        :value="buildState('watcher')"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <!-- <span class="core-process-item status-switch" @click.stop>
                        <el-switch :value="$utils.ifProcessRunning('md_' + accountItem.source_name, processStatus)" @change="handleMdSwitch($event, accountItem)"></el-switch>
                    </span> -->
                </div>
            </div>
        </div>
        <span slot="reference" :class="[currentStatus]">
            <i class="el-icon-s-operation"></i>
            主进程状态
        </span>
    </el-popover>
</template>
<script>
import { mapGetters, mapState } from 'vuex';
import { accountSource, sourceType } from '__gConfig/accountConfig';
import { statusConfig } from '__gConfig/statusConfig';
import { switchTd, switchMd } from '__io/actions/account';
import { ifProcessRunning } from '__gUtils/busiUtils';

export default {
    data(){
        let statusLevel = {};
        Object.keys(statusConfig || {}).map(key => {
            statusLevel[key] = statusConfig[key].level;
        })
        return {
            config: sourceType,
            statusLevel
        }
    },

    computed: {
        ...mapGetters({
            sourceList: 'getSourceList',
            
        }),

        ...mapState({
            accountList: state => state.ACCOUNT.accountList,
            mdTdState: state => state.ACCOUNT.mdTdState,
            processStatus: state => state.BASE.processStatus
        }),

        currentStatus(){
            const t = this;
            if(t.processStatus['master'] === undefined && t.processStatus['watcher'] === undefined){
                return 'color-gray'
            }
            if(!ifProcessRunning('master', t.processStatus) || !ifProcessRunning('watcher', t.processStatus)){
                t.$message.error('主进程断开，不可交易，请重启应用！', 0)
                return 'color-red'
            }
            return 'color-green'
        }
    },

    methods: {
        buildState(processId){
            const t = this;
            return t.processStatus[processId]
        }
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
.core-status-popover{
    box-shadow: 0px 0px 30px $bg
}
.core-status-content{
    max-width: 250px;
    .core-item{
        float: left;
        width: 245px;
        margin: 10px;
        .type-name{
            font-size: 16px;
            color: #fff;
            margin-bottom: 10px;
            padding-left: 5px;
        }
        .core-status{
            font-size: 14px;
            .core-process-item{
                display: inline-block;
                width: 80px;
                padding-left: 5px;
                color: $font_5;
                text-align: left;
                vertical-align: bottom;
                color: $font;
            }
            .core-process-item.core-process-title{
                width: 150px;
            }
            .source-name{
                width: 45px;
            }
            .core-status{
                padding-left: 30px;
                box-sizing: border-box;
            }
            .status-switch{
                width: 40px;
            }
        }
    }
    
}


</style>
