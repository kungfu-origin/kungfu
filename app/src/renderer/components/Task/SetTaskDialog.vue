<template>
     <el-dialog 
    width="640px" 
    :title="`${method === 'add'? '添加' : '设置'} 交易任务`"  
    :visible="visible" 
    :close-on-click-modal="false"
    :close-on-press-escape="true"
    @close="handleClose"
    @keyup.enter.native="handleSubmitSetting"
    id="set-task-dialog"
    >
        <el-tabs tab-position="left" size="mini" v-model="activeTabName">
            <el-tab-pane
                :disabled="method !== 'add' && currentActiveConfigKey !== item.key"
                v-for="(item, index) in configList"
                :key="item.key"
                :label="item.name"
                size="mini"
                :name="item.key"
            >
                <ExtConfigForm
                    ref="taskSettingForm"
                    v-model="postFormList[index]"
                    :configList="extendConfig(item)"
                    :uniKey="item.uniKey || ''"
                    labelWidth="140px"
                    :method="method"
                ></ExtConfigForm>
            </el-tab-pane>
        </el-tabs>        
        <div slot="footer" class="dialog-footer">
            <el-button @click="handleClose" size="mini">取 消</el-button>
            <el-button type="primary" size="mini" @click="handleSubmitSetting" id="confirm-set-account-btn">确 定</el-button>
        </div>
     </el-dialog>
</template>

<script>

import moment from 'moment';
import ExtConfigForm from '@/components/Base/ExtConfigForm';

export default {

    props: {
        value: {
            type: Object,
            default: () => ({})
        },

        currentActiveConfigKey: '',

        configList: {
            type: Array,
            default: () => []
        },

        visible: {
            type: Boolean,
            default: false
        },

        method: {
            type: String,
            default: 'add'
        },
    },

    components: {
        ExtConfigForm
    },

    data () {
        return {
            activeTabName: this.getActiveTabNameByProps(),
            postFormList: this.getPostFormListByProps()
        }
        
    },

    computed: {
        targetConfigIndex () {
            return this.configList.findIndex(item => item.key === this.activeTabName)
        },
    },

    methods: {
        handleClose () {
            this.$emit('update:visible', false);
            this.refreshData();
        },

        handleSubmitSetting () {
            this.$refs.taskSettingForm[this.targetConfigIndex].validate(valid => {
                if (valid) {
                    const postData = this.postFormList[this.targetConfigIndex]
                    this.$emit('confirm', JSON.stringify({
                        ...postData,
                        parentId: BigInt(+moment().valueOf()).toString()

                    }), this.activeTabName)
                    this.handleClose();
                }
            })
        },

        extendConfig (item) {
            return [ ...(item.config || []),    {
                    "key": "sim",
                    "name": "模拟执行",
                    "type": "bool",
                    "required": false,
                    "tip": "开启后会模拟执行（非实盘），同时需确认该交易任务支持模拟执行",
                    "default": false
                },
            ]
        },

        getActiveTabNameByProps () {
            const defaultKey = (this.configList[0] || {}).key || '';
            return this.currentActiveConfigKey ? this.currentActiveConfigKey : defaultKey
        },

        getPostFormListByProps () {
            const configIndexByKey = this.getConfigIndexByKey(this.currentActiveConfigKey);
            return this.configList.map((c, i) => {
                if (i === configIndexByKey) return this.value;
                return {}
            });
        },

        getConfigIndexByKey (key) {
            return this.configList.findIndex(c => c.key === key)
        },

        refreshData () {
            this.postForm = {};
        },
    }
}
</script>

<style lang="scss">
    
</style>