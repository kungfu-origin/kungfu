<template>
    <el-dialog 
    width="360px" 
    v-if="visible"
    :title="`切换${currentMdSourceAccount.source_name}行情源`"  
    :visible="visible" 
    :close-on-click-modal="false"
    @close="handleClose"
    >
        <el-radio-group v-model.trim="selectedMdAccountId" style="width: 100%">
            <el-row>
                <el-col :span="12" v-for="item in accountsFromSameSource" :key="item.account_id">
                    <el-radio 
                    :label="item.account_id"
                    >{{item.account_id.toAccountId()}}</el-radio>
                </el-col>
            </el-row>
            
        </el-radio-group>
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting">确 定</el-button>
        </div>
    </el-dialog>    
</template>

<script>
import {changeAccountMd} from '@/io/db/account';
export default {
    props: {
        currentMdSourceAccount: {
            type: Object,
            default: {}
        },
        
        visible: {
            type: Boolean,
            default: false,
        },

        accountsFromSameSource: {
            type: Array,
            default: []
        }
    },

    data(){
        const t = this;
        return{
            selectedMdAccountId: (t.currentMdSourceAccount || {}).account_id
        }
    },

    methods: {
        //关闭窗口的时候清空数据
        handleClose() {
            this.$emit('afterSetting')
            this.$emit('update:visible', false)
        },


        //确认更改
        async handleSubmitSetting(){
            const t = this;
            //check if the md source change
            const ifMdSourceChange = !t.accountsFromSameSource.filter(a => (a.account_id === t.selectedMdAccountId))[0].receive_md
            if(ifMdSourceChange){
                const oldId = (t.currentMdSourceAccount || {}).account_id;
                const newId = t.selectedMdAccountId;
                try{
                    await changeAccountMd(oldId, false)
                    await changeAccountMd(newId, true)
                    t.$store.dispatch('getAccountList')
                    t.$message.success('操作成功！')
                }catch(err){
                    t.$message.error(err.message || '操作失败！')
                }
                t.handleClose()
            }else{
                t.handleClose()
            }
        }
    }
}
</script>

