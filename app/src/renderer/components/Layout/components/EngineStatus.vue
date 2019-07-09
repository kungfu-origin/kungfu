<template>
    <el-popover
        placement="top-start"
        width="auto"
        trigger="click"
        popper-class="account-status-popover"
        >
        <div class="account-status-content">
            <div class="account-item" >
                <div class="type-name">行情</div>
                <div class="account-status" v-for="accountItem in sourceList" :key="accountItem.account_id">
                    <span class="account-process-item source-name">
                        <el-tag
                        v-if="(config[accountItem.source_name]||{}).typeName"
                        :type="config[accountItem.source_name].type" 
                        >
                            {{accountItem.source_name}}
                        </el-tag> 
                    </span>
                    <span class="account-process-item text-overflow" :title="accountItem.account_id.toAccountId()">
                        {{accountItem.account_id.toAccountId()}}
                    </span>
                    <span  class="account-process-item text-overflow" style="width: 81px;">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('md_' + accountItem.source_name, processStatus)"
                        :value="buildMdState(accountItem)"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="account-process-item status-switch" @click.stop>
                        <el-switch :value="$utils.ifProcessRunning('md_' + accountItem.source_name, processStatus)" @change="handleMdSwitch($event, accountItem)"></el-switch>
                    </span>
                </div>
            </div>
            <div class="account-item" >
                <div class="type-name">交易</div>
                <div class="account-status" v-for="accountItem in accountList" :key="accountItem.account_id">
                    <span class="account-process-item source-name">
                        <el-tag
                        v-if="(config[accountItem.source_name]||{}).typeName"
                        :type="config[accountItem.source_name].type" 
                        >
                            {{accountItem.source_name}}
                        </el-tag> 
                    </span>
                    <span class="account-process-item text-overflow" :title="accountItem.account_id.toAccountId()">
                        {{accountItem.account_id.toAccountId()}}
                    </span>
                    <span  class="account-process-item text-overflow " style="width: 81px;">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('td_' + accountItem.account_id, processStatus)"
                        :value="buildTdState(accountItem)"></tr-status>
                        <tr-status v-else></tr-status>
                    </span>
                    <span class="account-process-item status-switch">
                        <el-switch :value="$utils.ifProcessRunning('td_' + accountItem.account_id, processStatus)"
                        @change="handleTdSwitch($event, accountItem)"></el-switch>
                    </span>
                </div>
            </div>
        </div>
        <span slot="reference" :class="[currentStatus]">
            <i class="el-icon-s-operation"></i>
            账户状态
        </span>
    </el-popover>
</template>
<script>
import { mapGetters, mapState } from 'vuex';
import { accountSource, sourceType } from '__gConfig/accountConfig';
import { statusConfig } from '__gConfig/statusConfig';
import { switchTd, switchMd } from '__io/actions/account';

export default {
    data(){
        let statusLevel = {};
        Object.keys(statusConfig || {}).map(key => {
            statusLevel[key] = statusConfig[key].level;
        })
        return {
            config: sourceType,
            statusLevel,
            // processStatus: Object.freeze({})
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

        //展示最坏的情况
        //有一个没开，且开的状态都是ready的，显示灰色
        //有一个没开，且开的状态不都是ready的，以开的状态最严重的颜色为准
        //全开，且开的状态都是ready的，显示绿色
        //全开，且开的状态不都是ready的，以开的状态最严重的颜色为准
        //简而言之：以最差的为准
        currentStatus(){
            const t = this;
            let tdProcessReady = false;
            let mdProcessReady = false;
            let tdStatusReady = 0;
            let mdStatusReady = 0;

            t.accountList.map(a => {
                const tdProcessStatus = t.$utils.ifProcessRunning('td_' + a.account_id, t.processStatus)
                if(tdProcessStatus) tdProcessReady = true;
                const tdStatus = t.buildTdState(a)
                let level = t.statusLevel[tdStatus];
                if(tdProcessStatus){
                    if(tdStatusReady === 0 && level > 0) tdStatusReady = level;
                    (level < tdStatusReady) && (tdStatusReady = level);
                }

                if(a.receive_md === 1){
                    const mdProcessStatus = t.$utils.ifProcessRunning('md_' + a.source_name, t.processStatus)                                       
                    if (mdProcessStatus) mdProcessReady = true;  
                    const mdStatus = t.buildMdState(a)
                    let level = t.statusLevel[mdStatus];
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
        buildMdState(accountItem){
            const t = this;
            return (t.mdTdState[`md_${accountItem.source_name}`] || {}).state
        },

        buildTdState(accountItem){
            const t = this;
            return (t.mdTdState[`td_${accountItem.account_id}`] || {}).state
        },

        //Td开关
        handleTdSwitch(value, account) {
            const t = this;
            switchTd(account, value).then(({ type, message }) => t.$message[type](message))
        },

        //行情开关
        handleMdSwitch(value, account) {
            const t = this
            switchMd(account, value).then(({ type, message }) => t.$message[type](message))  
        },
    }
}
</script>
<style lang="scss">
@import "@/assets/scss/skin.scss";
.account-status-popover{
    box-shadow: 0px 0px 30px $bg
}
.account-status-content{
    max-width: 300px;
    .account-item{
        float: left;
        width: 290px;
        margin: 10px;
        .type-name{
            font-size: 16px;
            color: #fff;
            margin-bottom: 10px;
            padding-left: 5px;
        }
        .account-status{
            font-size: 14px;
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
                width: 45px;
            }
            .account-status{
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
