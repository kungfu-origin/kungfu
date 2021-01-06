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
        <ExtConfigForm
            ref="accountForm"
            v-model="postForm"
            :configList="(accountSource[source] || {}).config || []"
            :uniKeyValidator="validateAccountId"
            :uniKey="accountSourceKey"
            :method="method"
        ></ExtConfigForm>
    
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleCancel" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting" id="confirm-set-account-btn">确 定</el-button>
        </div>
    </el-dialog>
</template>

<script>

import ExtConfigForm from '@/components/Base/ExtConfigForm';

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
            default: 'xtp'
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

    components: {
        ExtConfigForm
    },
    
    data () {
        return {
            postForm: deepClone(this.value)
        }
    },

    mounted () {
        this.checkPostForm();
    },

    computed: {
        
        accountSourceConfig () {
            return this.accountSource[this.source] || {}
        },

        accountSourceKey () {
            return this.accountSourceConfig.key || ''
        },
    },

    methods:{

        handleCancel () {
            this.close()
        },

        //提交，同时更新accounts
        handleSubmitSetting () {
            this.$refs.accountForm.validate(valid => {
                if(valid) {
                    this.addSuccess(this.getAddUpdateTarget())
                }
            })
        },

        getAddUpdateTarget () {
            if(this.type === 'td') return this.addUpdateTd();
            else if(this.type === 'md') return this.addUpdateMd();
            else return Promise.resolve(true)
        },

        addUpdateTd () {
            const accountId = `${this.source}_${this.postForm[this.accountSourceKey]}`;
            const formValue = this.postForm || {};

            if(this.method == 'add') { //添加账户
                return addTd(accountId, JSON.stringify(formValue))
            } else{ //编辑账户
                return updateTdConfig(accountId, JSON.stringify(formValue))
            }
        },

        addUpdateMd () {
            const sourceName = this.accountSourceConfig.source || {};
            const formValue = this.postForm || {};
            
            if(this.method == 'add') { //添加账户
                return addMd(sourceName, JSON.stringify(formValue))
            } else{ //编辑账户
                return updateMdConfig(sourceName, JSON.stringify(formValue))
            }
        },

        addSuccess (thePromise) {
            return thePromise
            .then(() => {
                this.$emit('successSubmitSetting')
                this.$message.success('操作成功！')
            })
            .catch((err) => {
                this.$message.error(err.message || '操作失败！')
            })
            .finally(() => {
                this.close()
            })
        },

        checkPostForm () {
            const configItemList = this.accountSourceConfig.config || [];

            if(configItemList.length === 0) {
                if(this.type === 'td') {
                    this.$message.$error('账户配置出错！');
                    this.close();
                } else if(this.type === 'md') {
                    if(this.method === 'add') {
                        this.addSuccess(this.addUpdateMd())
                    } else {
                        this.$message.info('该行情源无需配置！')
                        this.close();
                    }
                }
                return;
            }
        },

        //关闭窗口的时候清空数据
        close () {
            this.$emit('update:visible', false)
            this.$emit('input', {})
            this.$emit('refreshData')
        },

        //检查account_id或者account_id是否重复
        validateAccountId (rule, value, callback) {
            if (this.type !== 'td') {
                callback()
                return;
            }

            const index = this.mdTdList.findIndex(a => {
               const account_id = a.account_id
               return account_id === `${this.source}_${value}`
            })
            if(index !== -1) {
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