<template>
    <el-dialog 
    width="540px" 
    :title=" (method == 'add' ? '添加' : '设置') + source + '柜台账户'"  
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
                    <el-input :class="item.key" v-if="item.type === 'str'" :type="item.key" v-model.trim="postForm[item.key]" :disabled="method == 'update' && (accountSource[source].key == item.key) && (type !== 'md')"></el-input>
                    <el-input :class="item.key" v-if="item.type === 'password'" :type="item.key" v-model.trim="postForm[item.key]" :disabled="method == 'update' && (accountSource[source].key == item.key) && (type !== 'md')" show-password></el-input>
                    <el-switch :class="item.key" v-if="item.type === 'bool'" :value="postForm[item.key]" @change="e => handleSwitchChange(item.key, e)"></el-switch>
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
import { addTd, updateTdConfig, addMd, updateMdConfig } from '__io/kungfu/account';

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
        //选中柜台下的所有账户，字段用于弹窗检查account_id或者account_id是否重名的
        mdTdList: {
            type: Array,
            default: () => []
        },

        accountSource: {
            type: Object,
            default: () => ({})
        },

        type: {
            type: String,
            default: 'td'
        }
    },
    data() {
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

    methods:{

        handleSwitchChange (key, e) {
            this.$set(this.postForm, key, e)
        },

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

                    let changeAccount;
                    if(t.type === 'td') changeAccount = t.addUpdateTd();
                    else if(t.type === 'md') changeAccount = t.addUpdateMd();

                    t.addSuccess(changeAccount)
                }
            })
        },

        addUpdateTd() {
            const t = this;
            const accountId = `${t.source}_${t.postForm[t.accountSource[t.source].key]}`;
            const formValue = t.postForm || {};

            if(t.method == 'add') { //添加账户
                return addTd(accountId, JSON.stringify(formValue))
            } else{ //编辑账户
                return updateTdConfig(accountId, JSON.stringify(formValue))
            }
        },

        addUpdateMd() {
            const t = this;
            const sourceName = t.accountSource[t.source].source;
            const formValue = t.postForm || {};
            
            if(t.method == 'add') { //添加账户
                return addMd(sourceName, JSON.stringify(formValue))
            } else{ //编辑账户
                return updateMdConfig(sourceName, JSON.stringify(formValue))
            }
        },

        addSuccess(thePromise) {
            const t = this;
            return thePromise
            .then(() => {
                t.$emit('successSubmitSetting')
                t.$message.success('操作成功！')
            })
            .catch((err) => {
                t.$message.error(err.message || '操作失败！')
            })
            .finally(() => {
                t.close()
            })
        },

        initPostForm() {
            const t = this;
            const configItemList = (t.accountSource[t.source].config || []);

            if(configItemList.length === 0) {
                if(t.type === 'td') {
                    t.$message.$error('账户配置出错！');
                    t.close();
                } else if(t.type === 'md') {
                    if(t.method === 'add') {
                        t.addSuccess(t.addUpdateMd())
                    } else {
                        t.$message.info('该行情源无需配置！')
                        t.close();
                    }
                }
                return;
            }

            configItemList.forEach(item => {
                const key = item.key;
                const type = item.type;
                const defaultVal = item.default;
                if((t.postForm[key] === undefined) || (t.postForm[key] === '')) {
                    if (type === 'int') {
                        if (!+defaultVal) {
                            t.$set(t.postForm, key, '')   
                        }
                    }
                    
                    if(item.default) {
                        t.$set(t.postForm, key, item.default)
                    } else {
                        t.$set(t.postForm, key, '')
                    }
                }
            })
        },

        buildValidators(item) {
            const t = this;
            let validators = [];
            if((t.method == 'add') && (t.type === 'td') && (t.accountSource[t.source].key == item.key)){
                validators = [
                    { validator: t.validateAccountId, trigger: 'blur' },
                ] 
            }

            if(item.validator && item.validator.length) {
                validators = [...validators, ...item.validator.map(v => ({validator: v, trigger: 'blur'}))]
            }

            if(item.required) validators.push({required: true, message: item.errMsg, trigger: 'blur'})
            
            return validators
        },

        //关闭窗口的时候清空数据
        close() {
            this.$emit('update:visible', false)
            this.$emit('input', {})
            this.$emit('refreshData')
        },

        //检查account_id或者account_id是否重复
        validateAccountId(rule, value, callback) {
            const t = this
            const index = t.mdTdList.findIndex(a => {
               const account_id = a.account_id
               return account_id == `${t.source}_${value}`
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