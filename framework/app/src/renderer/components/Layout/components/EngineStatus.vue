<template>
    <el-popover
        placement="top-start"
        width="auto"
        trigger="click"
        popper-class="kf-footer-popover"
        >
        <div class="account-status-content">
            <div class="account-item" >
                <div class="type-name">行情</div>
                <div class="account-status" v-for="accountItem in mdList" :key="accountItem.source_name">
                    <span class="account-process-item source-name">
                        <el-tag
                        v-if="(mdAccountSource[accountItem.source_name]||{}).typeName"
                        :type="mdAccountSource[accountItem.source_name].type" 
                        >
                            {{accountItem.source_name}}
                        </el-tag> 
                    </span>
                    <span class="account-process-item text-overflow"></span>
                    <span  class="account-process-item text-overflow" style="width: 81px;">
                        <tr-status 
                        v-if="ifProcessRunning(`md_${accountItem.source_name}`, processStatus) && processStatus[`md_${accountItem.source_name}`] === 'online'"
                        :value="(mdTdState[`md_${accountItem.source_name}`] || {}).state"></tr-status>
                        <tr-status 
                        v-else-if="ifProcessRunning(`md_${accountItem.source_name}`, processStatus) && processStatus[`md_${accountItem.source_name}`] === 'stopping'"
                        :value="processStatus[`md_${accountItem.source_name}`]"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="account-process-item text-overflow">
                        CPU:{{ getMemCpu(`md_${accountItem.source_name}`, processStatusWithDetail, 'cpu') }}
                    </span>
                    <span class="account-process-item text-overflow">
                        MEM:{{ getMemCpu(`md_${accountItem.source_name}`, processStatusWithDetail, 'memory') }}
                    </span>
                    <span class="account-process-item status-switch" @click.stop>
                        <el-switch :value="ifProcessRunning(`md_${accountItem.source_name}`, processStatus)" @change="handleMdSwitch($event, accountItem)"></el-switch>
                    </span>             
                     <span class="account-process-item status-switch" @click.stop="handleOpenLogFile(`md_${accountItem.source_name}`)">
                        <i class="el-icon-document mouse-over" title="打开日志文件"></i>
                    </span>
                </div>
            </div>
            <div class="account-item" >
                <div class="type-name">交易</div>
                <div class="account-status" v-for="accountItem in tdList" :key="accountItem.account_id">
                    <span class="account-process-item source-name">
                        <el-tag
                        v-if="(tdAccountSource[accountItem.source_name]||{}).typeName"
                        :type="tdAccountSource[accountItem.source_name].type" 
                        >
                            {{accountItem.source_name}}
                        </el-tag> 
                    </span>
                    <span class="account-process-item text-overflow" :title="accountItem.account_id.toAccountId()">
                        {{accountItem.account_id.toAccountId()}}
                    </span>
                    <span  class="account-process-item text-overflow " style="width: 81px;">
                        <tr-status 
                        v-if="ifProcessRunning(`td_${accountItem.account_id}`, processStatus) && processStatus[`td_${accountItem.account_id}`] === 'online'"
                        :value="(mdTdState[`td_${accountItem.account_id}`] || {}).state"></tr-status>
                        <tr-status 
                        v-else-if="ifProcessRunning(`td_${accountItem.account_id}`, processStatus) && processStatus[`td_${accountItem.account_id}`] === 'stopping'"
                        :value="processStatus[`td_${accountItem.account_id}`]"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="account-process-item text-overflow monit">
                        CPU:{{ getMemCpu(`td_${accountItem.account_id}`, processStatusWithDetail, 'cpu') }} 
                    </span>
                    <span class="account-process-item text-overflow monit">
                        MEM:{{ getMemCpu(`td_${accountItem.account_id}`, processStatusWithDetail, 'memory') }} 
                    </span>
                    <span class="account-process-item status-switch">
                        <el-switch :value="ifProcessRunning(`td_${accountItem.account_id}`, processStatus)"
                        @change="handleTdSwitch($event, accountItem)"></el-switch>
                    </span>
                    <span class="account-process-item status-switch" @click.stop="handleOpenLogFile(`td_${accountItem.account_id}`)">
                        <i class="el-icon-document mouse-over" title="打开日志文件"></i>
                    </span>
                    
                </div>
            </div>
        </div>
        <span slot="reference" :class="[currentStatus]">
            <i class="el-icon-s-operation"></i>
            账户进程状态
        </span>
    </el-popover>
</template>
<script>
import { mapState } from 'vuex';
import { ProcessStatusConfig } from '@kungfu-trader/kungfu-uicc/config/tradingConfig';
import { switchTd, switchMd } from '@kungfu-trader/kungfu-uicc/io/actions/account';
import { ifProcessRunning, getMemCpu } from '@kungfu-trader/kungfu-uicc/utils/busiUtils';

import openLogMixin from "__renderer/assets/mixins/openLogMixin";

export default {
    mixins: [ openLogMixin ],

    data () {
        let statusLevel = {};
        Object.keys(ProcessStatusConfig || {}).map(key => {
            statusLevel[key] = ProcessStatusConfig[key].level;
        })

        this.ifProcessRunning = ifProcessRunning;
        this.getMemCpu = getMemCpu;
        return {
            statusLevel
        }
    },

    computed: {
        ...mapState({
            tdList: state => state.ACCOUNT.tdList,
            mdList: state => state.ACCOUNT.mdList,
            mdTdState: state => state.ACCOUNT.mdTdState,
            processStatus: state => state.BASE.processStatus,
            processStatusWithDetail: state => state.BASE.processStatusWithDetail,
            tdAccountSource: state => (state.BASE.tdAccountSource || {}),
            mdAccountSource: state => (state.BASE.mdAccountSource || {})
        }),

        //展示最坏的情况
        //有一个没开，且开的状态都是ready的，显示灰色
        //有一个没开，且开的状态不都是ready的，以开的状态最严重的颜色为准
        //全开，且开的状态都是ready的，显示绿色
        //全开，且开的状态不都是ready的，以开的状态最严重的颜色为准
        //简而言之：以最差的为准
        currentStatus () {
            let tdProcessReady = false;
            let mdProcessReady = false;
            let tdStatusReady = 0;
            let mdStatusReady = 0;

            this.tdList.map(a => {
                const tdProcessStatus = this.ifProcessRunning('td_' + a.account_id, this.processStatus)
                if(tdProcessStatus) tdProcessReady = true;
                const tdStatus = this.buildTdState(a)
                let level = this.statusLevel[tdStatus];
                if(tdProcessStatus){
                    if(tdStatusReady === 0 && level > 0) tdStatusReady = level;
                    (level < tdStatusReady) && (tdStatusReady = level);
                }

                if(true){
                    const mdProcessStatus = this.ifProcessRunning('md_' + a.source_name, this.processStatus)                                       
                    if (mdProcessStatus) mdProcessReady = true;  
                    const mdStatus = this.buildMdState(a)
                    let level = this.statusLevel[mdStatus];
                    if(mdProcessStatus){
                        if(mdStatusReady === 0 && level > 0) mdStatusReady = level;
                        (level < mdStatusReady) && (mdStatusReady = level);
                    }
                }
            })

            if(mdProcessReady && tdProcessReady && tdStatusReady > 0 && mdStatusReady > 0 ){
                return 'color-green'
            }else if(mdStatusReady < 0 || tdStatusReady < 0){
               return 'color-red'
            }else{
                return 'color-gray'
            }

        }
    },

    methods: {
        buildMdState (accountItem) {
            return (this.mdTdState[`md_${accountItem.source_name}`] || {}).state
        },

        buildTdState (accountItem) {
            return (this.mdTdState[`td_${accountItem.account_id}`] || {}).state
        },

        //Td开关
        handleTdSwitch (value, account) {
            switchTd(account, value).then(({ type, message }) => this.$message[type](message))
        },

        //行情开关
        handleMdSwitch (value, account) {
            switchMd(account, value).then(({ type, message }) => this.$message[type](message))  
        },
    }
}
</script>
<style lang="scss">
@import "__renderer/assets/scss/skin.scss";

.account-status-content{
    display: flex;
    flex-direction: column;
    font-family: Consolas, Monaco, monospace,"Microsoft YaHei",sans-serif;

    .account-item{
        float: left;
        margin: 10px;

        .type-name{
            font-size: 16px;
            color: #fff;
            margin-bottom: 10px;
            padding-left: 5px;
        }

        .account-status{
            font-size: 14px;
            padding: 3px 0;
            display: flex;
            justify-content: space-around;
            
            .account-process-item{
                display: inline-block;
                width: 80px;
                padding-left: 5px;
                color: $font_5;
                text-align: left;
                vertical-align: bottom;
                color: $font;
            }

            .source-name{
                width: 60px;
            }

            .account-status{
                padding-left: 30px;
                box-sizing: border-box;
            }

            &.monit {
                width: 95px;
            }

            .status-switch{
                width: 40px;
            }
        }
    }
    
}


</style>
