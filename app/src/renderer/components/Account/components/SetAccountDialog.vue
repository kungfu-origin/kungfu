<template>
    <el-dialog 
    width="540px" 
    :title=" (method == 'add' ? '添加' : '设置') + source + '柜台账户'"  
    v-if="visible"
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleCancel"
    @keyup.enter.native="handleSubmitSetting"
    id="confirm-set-account-dialog"
    
    >
        <el-form 
        v-if="!!accountSource[source] && accountSource[source].config"        
        ref="accountForm" label-width="140px" :model="postForm">
            <!-- 自定义部分 -->
            <el-form-item 
            v-for="item of accountSource[source].config" :key="item.key"
            :label="item.name"
            :prop="item.key"
            :rules="buildValidators(item)"
            >
                <el-col :span="19">
                    <el-input :class="item.key" v-if="item.type === 'str'" :type="item.key" v-model.trim="postForm[item.key]" :disabled="method == 'update' && accountSource[source].key == item.key"></el-input>
                    <el-input :class="item.key" v-if="item.type === 'password'" :type="item.key" v-model.trim="postForm[item.key]" :disabled="method == 'update' && accountSource[source].key == item.key" show-password></el-input>
                    <el-switch :class="item.key" v-if="item.type === 'bool'" v-model.trim="postForm[item.key]"></el-switch>
                    <el-input-number :class="item.key" v-if="item.type === 'int'" :controls="false" v-model.trim="postForm[item.key]"></el-input-number>
                    <el-input-number :class="item.key" v-if="item.type === 'float'" :controls="false" v-model.trim="postForm[item.key]"></el-input-number>
                    <span class="account-setting-path path-selection-in-dialog text-overflow" v-if="item.type === 'file'" :title="postForm[item.key]">{{postForm[item.key]}}</span>                    
                    <el-button size="mini" icon="el-icon-more" v-if="item.type === 'file'" @click="handleSelectFile(item.key)"></el-button>
                    <el-select :class="item.key" size="small" v-if="item.type === 'select'" :multiple="item.multiple" collapse-tags  v-model.trim="postForm[item.key]" placeholder="请选择">
                        <el-option
                            v-for="option in item.data"
                            :key="option.value"
                            :label="option.name"
                            :value="option.value">
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
            <el-button type="primary" size="mini" @click="handleSubmitSetting" id="confirm-set-account-btn">确 定</el-button>
        </div>
    </el-dialog>
</template>

<script>
import { mapState } from 'vuex';
import * as ACCOUNT_API from '__io/db/account';
import { deepClone } from '__gUtils/busiUtils';
import { remote } from 'electron';

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
        //存放初始数据格式
        this.initData = {
            resolve_mode: 'auto',
            strategy_pnl_check: false,
            need_auth: false,
            need_settlement_confirm: false,
        }

        return {
            postForm: deepClone(this.value)
        }
    },

    mounted(){
       this.initPostForm();
    },

    watch: {
        postForm(){
            this.initPostForm();
        }
    },

    computed: {
        ...mapState({
            accountSource: state => state.BASE.accountSource
        })
    },

    methods:{
        handleCancel() {
            this.close()
        },

        //添加文件
        handleSelectFile(targetKey) {
            const t = this;
            const dialog = remote.dialog;
            dialog.showOpenDialog({
                properties: ['openFile']
            }, (filePath) => {
                if(!filePath || !filePath[0]) return;
                t.$set(t.postForm, targetKey, filePath[0]);
                t.$refs.accountForm.validate() //手动进行再次验证，因数据放在span中，改变数据后无法触发验证
            })
        },

        //提交，同时更新accounts
        handleSubmitSetting() {
            const t = this
            t.$refs.accountForm.validate(valid => {
                if(valid) {
                    let account_id = `${t.source}_${t.postForm[t.accountSource[t.source].key]}`
                    const formValue = t.postForm
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

        initPostForm() {
             const t = this;
            (t.accountSource[t.source].config || []).forEach(item => {
                const key = item.key;
                if((t.postForm[key] === undefined) || (t.postForm[key] === '')) {
                    if(item.default) {
                        t.postForm[key] = item.default
                    }
                }
            })
        },

        buildValidators(item) {
            const t = this;
            if(t.method == 'add' && t.accountSource[t.source].key == item.key){
                return [
                    {validator: t.validateAccountId, trigger: 'blur'},
                    {required: true, message: item.errMsg, trigger: 'blur'}
                ] 
            }else{
                let validators = [];
                if(item.validator && item.validator.length) validators = item.validator.map(v => ({validator: v, trigger: 'blur'}))
                if(item.required) validators.push({required: true, message: item.errMsg, trigger: 'blur'})
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

.path-selection-in-dialog.account-setting-path {
    width: 240px;
}

</style>