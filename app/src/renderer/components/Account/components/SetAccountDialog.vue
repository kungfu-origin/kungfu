<template>
    <el-dialog 
    width="540px" 
    :title=" (method == 'add' ? '添加' : '设置') + source + '柜台账户'"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    @close="handleCancel"
    >
        <el-form ref="accountForm" label-width="140px" :model="value">
            <!-- 自定义部分 -->
            <el-form-item 
            v-for="item of config[source]" :key="item.key"
            :label="item.name"
            :prop="item.key"
            :rules="buildValidators(item)"
            >
                <el-col :span="19">
                    <el-input v-if="item.type === 'str'" :type="item.key" v-model.trim="value[item.key]" :disabled="method == 'update' && sourceType[source].key == item.key"></el-input>
                    <el-input v-if="item.type === 'password'" :type="item.key" v-model.trim="value[item.key]" :disabled="method == 'update' && sourceType[source].key == item.key" show-password></el-input>
                    <el-switch v-if="item.type === 'bool'" v-model.trim="value[item.key]"></el-switch>
                    <el-input-number v-if="item.type === 'int'"  :controls="false" v-model.trim="value[item.key]"></el-input-number>
                    <el-select size="small" v-if="item.type === 'select'" :multiple="item.multiple" collapse-tags  v-model.trim="value[item.key]" placeholder="请选择">
                        <el-option
                            v-for="item in item.data"
                            :key="item.value"
                            :label="item.name"
                            :value="item.value">
                        </el-option>
                    </el-select>
                </el-col>
                <el-col :span="2" :offset="1" v-if="item.tip">
                    <el-tooltip class="item" effect="dark" :content="item.tip" placement="right">
                        <i class="el-icon-question mouse-over"></i>
                    </el-tooltip>
                </el-col>
            </el-form-item>

        </el-form>
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleCancel" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting">确 定</el-button>
        </div>
    </el-dialog>
</template>

<script>
import {accountSource, sourceType} from '@/assets/config/accountConfig';
import {mapState} from 'vuex';
import * as ACCOUNT_API from '@/io/db/account';
export default {
    name: 'set-account-dialog',
    props: {
        
        value: {},
        visible: {
            type: Boolean,
            default: false
        },
        //是该柜台下的第一个账户，则行情自动选中
        firstAccount: {
            type: Boolean,
            default: false
        },
        //属于哪个柜台的
        source: {
            type: String,
            default: 'oes'
        },
        //是添加还是设置
        method: {
            type: String,
            default: 'add'
        },
        //选中柜台下的所有账户，字段用于弹窗检查account_id或者user_id是否重名的
        accountList: {
            type: Array,
            default: () => []
        }
    },
    data() {
        this.sourceType = sourceType;
        this.config = accountSource;
        //存放初始数据格式
        this.initData = {
            resolve_mode: 'auto',
            strategy_pnl_check: false,
            need_auth: false,
            need_settlement_confirm: false,
        }
        return {}
    },

    methods:{
        handleCancel() {
            this.close()
        },

        //提交，同时更新accounts
        handleSubmitSetting() {
            const t = this
            t.$refs.accountForm.validate(valid => {
                if(valid) {
                    let account_id = `${t.source}_${t.value[t.sourceType[t.source].key]}`
                    const formValue = t.value
                    let changeAccount 
                    if(t.method == 'add') { //添加账户
                        changeAccount = ACCOUNT_API.addAccount(account_id, t.source, t.firstAccount, JSON.stringify(formValue))
                    } else{ //编辑账户
                        changeAccount = ACCOUNT_API.updateAccountConfig(account_id, JSON.stringify(formValue))
                    }

                    changeAccount.then(() => {
                        t.$emit('successSubmitSetting')
                        t.$message.success('操作成功！')
                    })
                    .catch((err) => {
                        t.$message.error(err.message || '操作失败！')
                    })
                    .finally(() => {
                        t.close()
                    })
                }
            })
        },

        buildValidators(item) {
            const t = this;
            if(t.method == 'add' && t.sourceType[t.source].key == item.key){
                return [
                    {validator: t.validateAccountId, trigger: 'blur'},
                    {required: true, message: item.rule, trigger: 'blur'}
                ] 
            }else{
                let validators = [];
                if(item.validator && item.validator.length) validators = item.validator.map(v => ({validator: v, trigger: 'blur'}))
                if(item.required) validators.push({required: true, message: item.rule, trigger: 'blur'})
                return validators
            }
        },

        //关闭窗口的时候清空数据
        close() {
            this.$emit('update:visible', false)
            this.$emit('input', {})
            this.$emit('refreshData')
        },

        //检查account_id或者user_id是否重复
        validateAccountId(rule, value, callback) {
            const t = this
            const index = t.accountList.findIndex(a => {
               const account_id = a.account_id.toAccountId();
               return account_id == value
            })
            if(index != -1) {
                callback('已存在该账户ID！')
            }else {
                callback()
            }
        }
    }
}
</script>

<style lang="scss">
@import '@/assets/scss/skin.scss';

</style>