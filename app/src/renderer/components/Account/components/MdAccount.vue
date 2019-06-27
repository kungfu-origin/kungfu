<template>
      <tr-dashboard title="行情源">
        <div class="md-source">
            <el-table
            size="small"
            :data="sourceList"
            height="100%"
            v-if="renderTable"
            >
                <el-table-column
                    prop="source_name"
                    label="柜台"
                    show-overflow-tooltip
                >
                    <template slot-scope="props">
                        <el-tag
                        v-if="(config[props.row.source_name] || {}).typeName"
                        :type="config[props.row.source_name].type" 
                        >
                            {{props.row.source_name}}
                        </el-tag>
                    </template>
                </el-table-column>
                <el-table-column
                    prop="account_id"
                    label="账户"
                    show-overflow-tooltip
                >
                    <template slot-scope="props">
                        {{props.row.account_id.toAccountId()}}
                    </template>
                </el-table-column>
                
                <el-table-column
                    label="状态"
                    show-overflow-tooltip
                    >
                    <template slot-scope="props">
                        <tr-status 
                        v-if="$utils.ifProcessRunning('md_' + props.row.source_name, processStatus)"
                        :value="(mdTdState[`md_${props.row.source_name}`] || {}).state"></tr-status>
                        <tr-status v-else></tr-status>
                    </template>
                </el-table-column>
                    
                <el-table-column
                    label="连接"
                    >
                    <template slot-scope="props">
                        <span @click.stop>
                            <el-switch :value="$utils.ifProcessRunning('md_' + props.row.source_name, processStatus)" @change="handleMdSwitch($event, props.row)"></el-switch>
                        </span>
                    </template>
                </el-table-column>
                <el-table-column
                    label=""
                    align="right"
                >
                    <template slot-scope="props">
                        <span class="tr-oper" @click.stop="handleOpenLogFile(props.row)"><i class="el-icon-document mouse-over" title="打开日志文件"></i></span>
                        <span class="tr-oper" @click.stop="handleUpdateMdSource(props.row)" title="切换行情源"><i class="el-icon-s-data mouse-over"></i></span>
                    </template>
                </el-table-column>
            </el-table>
        </div>
        <SetMdSourceDialog 
        :visible.sync="setMdSourceDialogVisiblity"
        :currentMdSourceAccount="currentMdSourceAccount"
        :accountsFromSameSource="accountList.filter(a => (a.source_name === (currentMdSourceAccount || {}).source_name))"
        @afterSetting="getAccountList"
        ></SetMdSourceDialog>
    </tr-dashboard>
</template>

<script>
import path from 'path';
import Vue from 'vue';
import { mapState, mapGetters } from 'vuex';
import { sourceType } from '@/assets/config/accountConfig'
import * as ACCOUNT_API from '@/io/db/account';
import { openReadFile } from '__gUtils/fileUtils';
import { LOG_DIR } from '__gConfig/pathConfig';
import { switchMd } from '@/io/actions/account';
import SetMdSourceDialog from './SetMdSourceDialog';

export default {
    data(){
        return {
            config: sourceType,
            setMdSourceDialogVisiblity: false,
            currentMdSourceAccount: null,
            renderTable: false
        }
    },

    computed: {
        ...mapState({
            accountList: state => state.ACCOUNT.accountList,
            mdTdState: state => state.ACCOUNT.mdTdState,
            processStatus: state => state.BASE.processStatus
        }),

        ...mapGetters({
            sourceList: 'getSourceList'
        }) 
    },

    components: {
        SetMdSourceDialog
    },

    mounted(){
        const t = this;
        t.renderTable = true;
    },

    methods: {
        //切换行情源
        handleUpdateMdSource(mdSourceAccount){
            const t = this;
            if(t.$utils.ifProcessRunning('md_' + mdSourceAccount.source_name, t.processStatus)){
                t.$message.warning("需先停止当前行情运行！")
            }else{
                t.currentMdSourceAccount = mdSourceAccount;
                t.setMdSourceDialogVisiblity = true;
            }
        },

        //行情开关
        handleMdSwitch(value, account) {
            const t = this
            switchMd(account, value).then(({ type, message }) => t.$message[type](message))  
        },

        handleOpenLogFile(row){
            const logPath = path.join(LOG_DIR, `md_${row.source_name}.log`);
            openReadFile(logPath);
        },

        //获取账户列表
        getAccountList() {
            const t = this;
            ACCOUNT_API.getAccountList().then(res => {
                if(!res) return;
                t.$store.dispatch('setAccountList', res)
            }).catch(err => {
                t.$message.error(err.message || '操作失败！')
            })
        },
    }
    
    
}
</script>
<style lang="scss">
@import '@/assets/scss/skin.scss';
.md-source{
   height: 100%;
}

</style>





